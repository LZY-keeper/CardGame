#include "controllers/GameController.h"

#include "configs/loaders/LevelConfigLoader.h"
#include "configs/models/LevelConfig.h"
#include "services/CardRuleService.h"
#include "services/GameModelFromLevelGenerator.h"
#include "views/GameView.h"

namespace cardgame {

GameController::GameController()
    : _gameView(nullptr)
    , _levelConfigPath()
    , _isAnimating(false)
    , _isGameFinished(false) {
}

GameController::~GameController() = default;

bool GameController::init(GameView* gameView, const std::string& levelConfigPath) {
    if (gameView == nullptr) {
        return false;
    }

    _gameView = gameView;
    _levelConfigPath = levelConfigPath;
    bindViewCallbacks();
    restartLevel();
    return true;
}

void GameController::bindViewCallbacks() {
    _gameView->setOnPlayfieldCardClick([this](int cardId) {
        handlePlayfieldCardClick(cardId);
    });
    _gameView->setOnDrawPileClick([this]() {
        handleDrawPileClick();
    });
    _gameView->setOnUndoClick([this]() {
        handleUndoClick();
    });
    _gameView->setOnRestartClick([this]() {
        handleRestartClick();
    });
}

void GameController::restartLevel() {
    LevelConfig levelConfig;
    if (!LevelConfigLoader::loadLevelConfig(_levelConfigPath, &levelConfig)) {
        return;
    }
    if (!GameModelFromLevelGenerator::generateGameModel(levelConfig, &_gameModel)) {
        return;
    }

    _undoManager.clear();
    _isAnimating = false;
    _isGameFinished = false;
    _gameView->hideResultOverlay();
    _gameView->setInteractionEnabled(true);
    _gameView->presentGame(_gameModel);
    _gameView->setUndoEnabled(false);
}

void GameController::finalizeMutation(const std::vector<int>& revealedCardIds) {
    _isAnimating = false;

    if (isVictory()) {
        _isGameFinished = true;
        _gameView->syncGame(_gameModel);
        _gameView->setInteractionEnabled(false);
        _gameView->setUndoEnabled(false);
        _gameView->showResultOverlay(true, calculateVictoryStars(), static_cast<int>(_gameModel.drawPileCardIds.size()));
        return;
    }

    if (isDefeat()) {
        _isGameFinished = true;
        _gameView->syncGame(_gameModel);
        _gameView->setInteractionEnabled(false);
        _gameView->setUndoEnabled(false);
        _gameView->showResultOverlay(false, 0, 0);
        return;
    }

    _gameView->setInteractionEnabled(true);
    _gameView->syncGame(_gameModel);
    if (!revealedCardIds.empty()) {
        _gameView->playRevealAnimations(revealedCardIds);
    }

    _gameView->setUndoEnabled(_undoManager.canUndo());
}

void GameController::handlePlayfieldCardClick(int cardId) {
    if (_isAnimating || _isGameFinished) {
        return;
    }

    CardModel* selectedCard = _gameModel.getCardById(cardId);
    const CardModel* trayCard = _gameModel.getCardById(_gameModel.getCurrentTrayCardId());
    if (selectedCard == nullptr || trayCard == nullptr ||
        !selectedCard->isPlayfieldCard() || !selectedCard->isFaceUp) {
        return;
    }
    if (!CardRuleService::canMatch(selectedCard->cardFace, trayCard->cardFace)) {
        return;
    }

    const GameModel previousModel = _gameModel;
    _undoManager.pushRecord(UndoModel(UAT_MATCH_PLAYFIELD, cardId, _gameModel));
    _isAnimating = true;
    _gameView->setInteractionEnabled(false);
    _gameView->playCardMoveToTray(cardId, [this, cardId, previousModel]() {
        CardModel* movingCard = _gameModel.getCardById(cardId);
        if (movingCard != nullptr) {
            movingCard->areaType = CAT_TRAY;
            movingCard->isFaceUp = true;
            _gameModel.trayHistoryCardIds.push_back(cardId);
            _gameModel.refreshPlayfieldFaceUpState();
        }
        finalizeMutation(collectNewlyFaceUpCardIds(previousModel));
    });
}

void GameController::handleDrawPileClick() {
    if (_isAnimating || _isGameFinished) {
        return;
    }

    const int topDrawCardId = _gameModel.getTopDrawPileCardId();
    if (topDrawCardId < 0) {
        return;
    }

    _undoManager.pushRecord(UndoModel(UAT_DRAW_FROM_STACK, topDrawCardId, _gameModel));
    _isAnimating = true;
    _gameView->setInteractionEnabled(false);
    _gameView->playCardMoveToTray(topDrawCardId, [this, topDrawCardId]() {
        CardModel* drawCard = _gameModel.getCardById(topDrawCardId);
        if (drawCard != nullptr) {
            drawCard->areaType = CAT_TRAY;
            drawCard->isFaceUp = true;
            if (!_gameModel.drawPileCardIds.empty() && _gameModel.drawPileCardIds.front() == topDrawCardId) {
                _gameModel.drawPileCardIds.erase(_gameModel.drawPileCardIds.begin());
            }
            _gameModel.trayHistoryCardIds.push_back(topDrawCardId);
        }
        finalizeMutation(std::vector<int>());
    });
}

void GameController::handleUndoClick() {
    if (_isAnimating || _isGameFinished || !_undoManager.canUndo()) {
        return;
    }

    const UndoModel undoRecord = _undoManager.popRecord();
    if (undoRecord.movedCardId < 0) {
        _gameView->setUndoEnabled(_undoManager.canUndo());
        return;
    }

    _isAnimating = true;
    _gameView->setInteractionEnabled(false);
    _gameModel = undoRecord.snapshot;
    _gameView->hideResultOverlay();
    _gameView->syncGame(_gameModel, undoRecord.movedCardId);
    _gameView->playCardMoveToPresentation(undoRecord.movedCardId, _gameModel, [this]() {
        _isAnimating = false;
        _isGameFinished = false;
        _gameView->setInteractionEnabled(true);
        _gameView->syncGame(_gameModel);
        _gameView->setUndoEnabled(_undoManager.canUndo());
    });
}

void GameController::handleRestartClick() {
    if (_isAnimating) {
        return;
    }
    restartLevel();
}

std::vector<int> GameController::collectNewlyFaceUpCardIds(const GameModel& previousModel) const {
    std::vector<int> revealedCardIds;
    for (size_t index = 0; index < _gameModel.cards.size(); ++index) {
        const CardModel& currentCard = _gameModel.cards[index];
        if (!currentCard.isPlayfieldCard() || !currentCard.isFaceUp) {
            continue;
        }

        const CardModel* previousCard = previousModel.getCardById(currentCard.cardId);
        if (previousCard != nullptr && !previousCard->isFaceUp) {
            revealedCardIds.push_back(currentCard.cardId);
        }
    }
    return revealedCardIds;
}

bool GameController::hasAnyPlayableCards() const {
    const CardModel* trayCard = _gameModel.getCardById(_gameModel.getCurrentTrayCardId());
    if (trayCard == nullptr) {
        return false;
    }

    for (size_t index = 0; index < _gameModel.cards.size(); ++index) {
        const CardModel& cardModel = _gameModel.cards[index];
        if (cardModel.isPlayfieldCard() &&
            cardModel.isFaceUp &&
            CardRuleService::canMatch(cardModel.cardFace, trayCard->cardFace)) {
            return true;
        }
    }

    return false;
}

bool GameController::isVictory() const {
    for (size_t index = 0; index < _gameModel.cards.size(); ++index) {
        if (_gameModel.cards[index].isPlayfieldCard()) {
            return false;
        }
    }
    return true;
}

bool GameController::isDefeat() const {
    return !isVictory() &&
           _gameModel.drawPileCardIds.empty() &&
           !hasAnyPlayableCards();
}

int GameController::calculateVictoryStars() const {
    const int remainingDrawCards = static_cast<int>(_gameModel.drawPileCardIds.size());
    if (remainingDrawCards >= 4) {
        return 3;
    }
    if (remainingDrawCards >= 2) {
        return 2;
    }
    return 1;
}

}  // namespace cardgame
