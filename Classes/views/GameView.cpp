#include "views/GameView.h"

#include <algorithm>

#include "2d/CCActionEase.h"
#include "2d/CCActionInstant.h"
#include "2d/CCActionInterval.h"
#include "2d/CCLabel.h"
#include "2d/CCLayer.h"
#include "2d/CCSprite.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerTouch.h"
#include "controllers/GameController.h"
#include "configs/models/CardResConfig.h"
#include "views/CardView.h"

namespace cardgame {

namespace {

const cocos2d::Size kDesignSize(1080.0f, 2080.0f);
const float kAnimationDuration = 0.28f;
const cocos2d::Vec2 kDrawPileAnchor(186.0f, 116.0f);
const cocos2d::Vec2 kTrayAnchor(626.0f, 116.0f);
const float kDrawPileOffsetX = 18.0f;
const float kDrawPileOffsetY = 5.0f;
const float kTrayHistoryOffsetX = 44.0f;
const int kMaxVisibleDrawPileCards = 6;
const int kMaxVisibleTrayCards = 3;

}  // namespace

cocos2d::Scene* GameView::createScene() {
    return GameView::create();
}

GameView::~GameView() = default;

bool GameView::init() {
    if (!Scene::init()) {
        return false;
    }

    _cardLayer = nullptr;
    _drawPileTapProxy = nullptr;
    _undoButton = nullptr;
    _drawCountLabel = nullptr;
    _undoLabel = nullptr;
    _progressLabel = nullptr;
    _statusLabel = nullptr;
    _resultOverlay = nullptr;
    _resultTitleLabel = nullptr;
    _resultBodyLabel = nullptr;
    _resultStarsLabel = nullptr;
    _isInteractionEnabled = true;
    _isUndoEnabled = false;
    _isResultVisible = false;

    createStaticUi();

    _gameController.reset(new GameController());
    return _gameController != nullptr &&
           _gameController->init(this, "configs/levels/level_1.json");
}

void GameView::setOnPlayfieldCardClick(const std::function<void(int)>& callback) {
    _onPlayfieldCardClick = callback;
}

void GameView::setOnDrawPileClick(const std::function<void()>& callback) {
    _onDrawPileClick = callback;
}

void GameView::setOnUndoClick(const std::function<void()>& callback) {
    _onUndoClick = callback;
}

void GameView::setOnRestartClick(const std::function<void()>& callback) {
    _onRestartClick = callback;
}

void GameView::presentGame(const GameModel& gameModel) {
    hideResultOverlay();
    syncGame(gameModel);
    playEntranceAnimation(gameModel);
}

void GameView::syncGame(const GameModel& gameModel, int excludedCardId) {
    for (size_t index = 0; index < gameModel.cards.size(); ++index) {
        const CardModel& cardModel = gameModel.cards[index];
        CardView* cardView = ensureCardView(cardModel);
        if (cardView == nullptr || cardModel.cardId == excludedCardId) {
            continue;
        }

        cocos2d::Vec2 targetPosition;
        float targetScale = 1.0f;
        bool showFaceUp = false;
        bool isInteractive = false;
        int zOrder = 0;
        const bool isVisible = getCardPresentation(
            gameModel,
            cardModel.cardId,
            &targetPosition,
            &targetScale,
            &showFaceUp,
            &isInteractive,
            &zOrder);

        cardView->setVisible(isVisible);
        if (!isVisible) {
            cardView->setInteractive(false);
            continue;
        }

        GLubyte opacity = 255;
        if (cardModel.isDrawPileCard()) {
            const std::vector<int>::const_iterator iterator =
                std::find(gameModel.drawPileCardIds.begin(), gameModel.drawPileCardIds.end(), cardModel.cardId);
            const int drawIndex = static_cast<int>(iterator - gameModel.drawPileCardIds.begin());
            opacity = static_cast<GLubyte>(std::max(120, 255 - drawIndex * 18));
        } else if (cardModel.isTrayCard()) {
            const std::vector<int>::const_iterator iterator =
                std::find(gameModel.trayHistoryCardIds.begin(), gameModel.trayHistoryCardIds.end(), cardModel.cardId);
            const int trayIndex = static_cast<int>(iterator - gameModel.trayHistoryCardIds.begin());
            const int fromTail = static_cast<int>(gameModel.trayHistoryCardIds.size()) - trayIndex - 1;
            opacity = static_cast<GLubyte>(fromTail == 0 ? 255 : std::max(138, 255 - fromTail * 52));
        }

        cardView->setPosition(targetPosition);
        cardView->setCardScale(targetScale);
        cardView->setLocalZOrder(zOrder);
        cardView->setOpacity(opacity);
        cardView->refresh(cardModel, showFaceUp, isInteractive && _isInteractionEnabled && !_isResultVisible);
    }

    updateDrawPileDecorations(gameModel);
    updateUndoButtonVisual();
}

bool GameView::getCardPresentation(
    const GameModel& gameModel,
    int cardId,
    cocos2d::Vec2* outPosition,
    float* outScale,
    bool* outFaceUp,
    bool* outInteractive,
    int* outZOrder) const {
    const CardModel* cardModel = gameModel.getCardById(cardId);
    if (cardModel == nullptr) {
        return false;
    }

    if (cardModel->isPlayfieldCard()) {
        *outPosition = cardModel->position;
        *outScale = CardResConfig::getPlayfieldScale();
        *outFaceUp = cardModel->isFaceUp;
        *outInteractive = cardModel->isFaceUp;
        *outZOrder = 240 + static_cast<int>(cardModel->position.y);
        return true;
    }

    if (cardModel->isDrawPileCard()) {
        const std::vector<int>::const_iterator iterator =
            std::find(gameModel.drawPileCardIds.begin(), gameModel.drawPileCardIds.end(), cardId);
        if (iterator == gameModel.drawPileCardIds.end()) {
            return false;
        }

        const int drawIndex = static_cast<int>(iterator - gameModel.drawPileCardIds.begin());
        if (drawIndex >= kMaxVisibleDrawPileCards) {
            return false;
        }

        *outPosition = kDrawPileAnchor + cocos2d::Vec2(-kDrawPileOffsetX * drawIndex, kDrawPileOffsetY * drawIndex);
        *outScale = CardResConfig::getTrayScale();
        *outFaceUp = false;
        *outInteractive = false;
        *outZOrder = 520 - drawIndex;
        return true;
    }

    if (cardModel->isTrayCard()) {
        const std::vector<int>::const_iterator iterator =
            std::find(gameModel.trayHistoryCardIds.begin(), gameModel.trayHistoryCardIds.end(), cardId);
        if (iterator == gameModel.trayHistoryCardIds.end()) {
            return false;
        }

        const int trayIndex = static_cast<int>(iterator - gameModel.trayHistoryCardIds.begin());
        const int fromTail = static_cast<int>(gameModel.trayHistoryCardIds.size()) - trayIndex - 1;
        if (fromTail >= kMaxVisibleTrayCards) {
            return false;
        }

        *outPosition = kTrayAnchor + cocos2d::Vec2(-kTrayHistoryOffsetX * fromTail, 0.0f);
        *outScale = CardResConfig::getTrayScale();
        *outFaceUp = true;
        *outInteractive = false;
        *outZOrder = 620 + trayIndex;
        return true;
    }

    return false;
}

void GameView::playCardMoveToTray(int cardId, const std::function<void()>& onComplete) {
    std::unordered_map<int, CardView*>::iterator iterator = _cardViews.find(cardId);
    if (iterator == _cardViews.end() || iterator->second == nullptr) {
        if (onComplete) {
            onComplete();
        }
        return;
    }

    iterator->second->setOpacity(255);
    iterator->second->setLocalZOrder(9999);
    iterator->second->playMoveTo(
        getTrayAnchor(),
        CardResConfig::getTrayScale(),
        kAnimationDuration,
        onComplete);
}

void GameView::playCardMoveToPresentation(
    int cardId,
    const GameModel& targetModel,
    const std::function<void()>& onComplete) {
    std::unordered_map<int, CardView*>::iterator iterator = _cardViews.find(cardId);
    if (iterator == _cardViews.end() || iterator->second == nullptr) {
        if (onComplete) {
            onComplete();
        }
        return;
    }

    cocos2d::Vec2 targetPosition;
    float targetScale = 1.0f;
    bool showFaceUp = false;
    bool isInteractive = false;
    int zOrder = 0;
    if (!getCardPresentation(
            targetModel,
            cardId,
            &targetPosition,
            &targetScale,
            &showFaceUp,
            &isInteractive,
            &zOrder)) {
        if (onComplete) {
            onComplete();
        }
        return;
    }

    const CardModel* targetCardModel = targetModel.getCardById(cardId);
    iterator->second->setVisible(true);
    iterator->second->setOpacity(255);
    iterator->second->setLocalZOrder(9999);
    iterator->second->refresh(*targetCardModel, showFaceUp, false);
    iterator->second->playMoveTo(targetPosition, targetScale, kAnimationDuration, onComplete);
}

void GameView::playRevealAnimations(const std::vector<int>& cardIds) {
    for (size_t index = 0; index < cardIds.size(); ++index) {
        std::unordered_map<int, CardView*>::iterator iterator = _cardViews.find(cardIds[index]);
        if (iterator != _cardViews.end() && iterator->second != nullptr && iterator->second->isVisible()) {
            iterator->second->playRevealAnimation();
        }
    }
}

void GameView::setInteractionEnabled(bool isEnabled) {
    _isInteractionEnabled = isEnabled;
    if (!isEnabled) {
        for (std::unordered_map<int, CardView*>::iterator iterator = _cardViews.begin();
             iterator != _cardViews.end();
             ++iterator) {
            if (iterator->second != nullptr) {
                iterator->second->setInteractive(false);
            }
        }
    }
}

void GameView::setUndoEnabled(bool isEnabled) {
    _isUndoEnabled = isEnabled;
    updateUndoButtonVisual();
}

void GameView::hideResultOverlay() {
    _isResultVisible = false;
    if (_resultOverlay != nullptr) {
        _resultOverlay->stopAllActions();
        _resultOverlay->setVisible(false);
    }
}

void GameView::showResultOverlay(bool isVictory, int starCount, int remainingDrawCards) {
    if (_resultOverlay == nullptr) {
        return;
    }

    _isResultVisible = true;
    _resultOverlay->setVisible(true);
    _resultOverlay->setOpacity(0);

    if (_resultTitleLabel != nullptr) {
        _resultTitleLabel->setString(isVictory ? "Well Done!" : "Try Again");
    }
    if (_resultBodyLabel != nullptr) {
        _resultBodyLabel->setString(isVictory
            ? cocos2d::StringUtils::format("All cards cleared.\nDraw cards left: %d", remainingDrawCards)
            : "No playable cards remain.\nTap restart to try again.");
    }
    if (_resultStarsLabel != nullptr) {
        if (isVictory) {
            std::string starLine;
            for (int index = 0; index < 3; ++index) {
                starLine += index < starCount ? "* " : ". ";
            }
            _resultStarsLabel->setString(starLine);
            _resultStarsLabel->setVisible(true);
        } else {
            _resultStarsLabel->setVisible(false);
        }
    }

    cocos2d::Node* resultPanel = _resultOverlay->getChildByTag(301);
    if (resultPanel != nullptr) {
        resultPanel->stopAllActions();
        resultPanel->setScale(0.84f);
        resultPanel->runAction(cocos2d::EaseBackOut::create(cocos2d::ScaleTo::create(0.2f, 1.0f)));
    }

    _resultOverlay->runAction(cocos2d::FadeTo::create(0.16f, 255));
}

cocos2d::Vec2 GameView::getTrayAnchor() const {
    return kTrayAnchor;
}

void GameView::createStaticUi() {
    cocos2d::LayerColor* baseLayer = cocos2d::LayerColor::create(cocos2d::Color4B(8, 14, 30, 255));
    addChild(baseLayer, 0);

    cocos2d::Sprite* backgroundSprite = cocos2d::Sprite::create("res/background.jpg");
    if (backgroundSprite != nullptr) {
        backgroundSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        backgroundSprite->setPosition(cocos2d::Vec2(kDesignSize.width * 0.5f, kDesignSize.height * 0.5f));
        const float scaleX = kDesignSize.width / backgroundSprite->getContentSize().width;
        const float scaleY = kDesignSize.height / backgroundSprite->getContentSize().height;
        backgroundSprite->setScale(std::max(scaleX, scaleY));
        addChild(backgroundSprite, 1);
    }

    cocos2d::LayerGradient* atmosphereOverlay = cocos2d::LayerGradient::create(
        cocos2d::Color4B(6, 15, 44, 20),
        cocos2d::Color4B(5, 10, 28, 195),
        cocos2d::Vec2(0.0f, -1.0f));
    addChild(atmosphereOverlay, 2);

    cocos2d::LayerGradient* bottomMist = cocos2d::LayerGradient::create(
        cocos2d::Color4B(255, 244, 220, 18),
        cocos2d::Color4B(255, 244, 220, 95),
        cocos2d::Vec2(0.0f, 1.0f));
    bottomMist->setContentSize(cocos2d::Size(kDesignSize.width, 700.0f));
    bottomMist->setIgnoreAnchorPointForPosition(false);
    bottomMist->setAnchorPoint(cocos2d::Vec2::ZERO);
    bottomMist->setPosition(cocos2d::Vec2::ZERO);
    addChild(bottomMist, 3);

    _topHudPanel = cocos2d::Node::create();
    _topHudPanel->setPosition(cocos2d::Vec2(65.0f, 1918.0f));
    addChild(_topHudPanel, 10);

    cocos2d::LayerColor* topHudBackground =
        cocos2d::LayerColor::create(cocos2d::Color4B(12, 28, 62, 168), 950.0f, 116.0f);
    _topHudPanel->addChild(topHudBackground);

    cocos2d::Label* titleLabel = cocos2d::Label::createWithSystemFont("Sky Palace Match", "Arial", 42);
    titleLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_TOP);
    titleLabel->setPosition(cocos2d::Vec2(475.0f, 94.0f));
    titleLabel->setTextColor(cocos2d::Color4B(255, 237, 186, 255));
    _topHudPanel->addChild(titleLabel);

    _statusLabel = cocos2d::Label::createWithSystemFont("Draw: --", "Arial", 28);
    _statusLabel->setAnchorPoint(cocos2d::Vec2(0.0f, 0.5f));
    _statusLabel->setPosition(cocos2d::Vec2(42.0f, 34.0f));
    _statusLabel->setTextColor(cocos2d::Color4B(230, 242, 255, 235));
    _topHudPanel->addChild(_statusLabel);

    _progressLabel = cocos2d::Label::createWithSystemFont("Progress: --", "Arial", 28);
    _progressLabel->setAnchorPoint(cocos2d::Vec2(1.0f, 0.5f));
    _progressLabel->setPosition(cocos2d::Vec2(908.0f, 34.0f));
    _progressLabel->setTextColor(cocos2d::Color4B(230, 242, 255, 235));
    _topHudPanel->addChild(_progressLabel);

    _bottomPanel = cocos2d::Node::create();
    _bottomPanel->setPosition(cocos2d::Vec2(70.0f, 58.0f));
    addChild(_bottomPanel, 100);

    cocos2d::LayerColor* bottomPanelBackground =
        cocos2d::LayerColor::create(cocos2d::Color4B(13, 25, 56, 188), 940.0f, 382.0f);
    _bottomPanel->addChild(bottomPanelBackground);

    cocos2d::Label* drawTitleLabel = cocos2d::Label::createWithSystemFont("DRAW PILE", "Arial", 30);
    drawTitleLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    drawTitleLabel->setPosition(cocos2d::Vec2(180.0f, 314.0f));
    drawTitleLabel->setTextColor(cocos2d::Color4B(255, 239, 195, 255));
    _bottomPanel->addChild(drawTitleLabel);

    cocos2d::Label* trayTitleLabel = cocos2d::Label::createWithSystemFont("CURRENT CARD", "Arial", 30);
    trayTitleLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    trayTitleLabel->setPosition(cocos2d::Vec2(644.0f, 314.0f));
    trayTitleLabel->setTextColor(cocos2d::Color4B(255, 239, 195, 255));
    _bottomPanel->addChild(trayTitleLabel);

    cocos2d::LayerColor* drawSlot = cocos2d::LayerColor::create(cocos2d::Color4B(255, 255, 255, 18), 220.0f, 238.0f);
    drawSlot->setPosition(cocos2d::Vec2(72.0f, 50.0f));
    _bottomPanel->addChild(drawSlot);

    cocos2d::LayerColor* traySlot = cocos2d::LayerColor::create(cocos2d::Color4B(255, 255, 255, 18), 276.0f, 238.0f);
    traySlot->setPosition(cocos2d::Vec2(482.0f, 50.0f));
    _bottomPanel->addChild(traySlot);

    _drawCountLabel = cocos2d::Label::createWithSystemFont("Remaining: 0", "Arial", 28);
    _drawCountLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    _drawCountLabel->setPosition(cocos2d::Vec2(180.0f, 16.0f));
    _drawCountLabel->setTextColor(cocos2d::Color4B(221, 240, 255, 255));
    _bottomPanel->addChild(_drawCountLabel, 4);

    _drawPileTapProxy = cocos2d::LayerColor::create(cocos2d::Color4B(255, 255, 255, 0), 236.0f, 250.0f);
    _drawPileTapProxy->setPosition(cocos2d::Vec2(60.0f, 42.0f));
    _bottomPanel->addChild(_drawPileTapProxy, 6);
    bindTapToNode(_drawPileTapProxy, [this]() {
        if (_isInteractionEnabled && _onDrawPileClick && !_isResultVisible) {
            _onDrawPileClick();
        }
    });

    _undoButton = cocos2d::LayerColor::create(cocos2d::Color4B(255, 209, 92, 220), 168.0f, 78.0f);
    _undoButton->setPosition(cocos2d::Vec2(736.0f, 34.0f));
    _bottomPanel->addChild(_undoButton, 6);

    _undoLabel = cocos2d::Label::createWithSystemFont("UNDO", "Arial", 30);
    _undoLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _undoLabel->setPosition(cocos2d::Vec2(820.0f, 73.0f));
    _undoLabel->setTextColor(cocos2d::Color4B(75, 42, 18, 255));
    _bottomPanel->addChild(_undoLabel, 7);

    bindTapToNode(_undoButton, [this]() {
        if (_isInteractionEnabled && _isUndoEnabled && _onUndoClick && !_isResultVisible) {
            _onUndoClick();
        }
    });

    _cardLayer = cocos2d::Node::create();
    addChild(_cardLayer, 110);

    createResultOverlay();
}

void GameView::createResultOverlay() {
    _resultOverlay = cocos2d::Node::create();
    _resultOverlay->setVisible(false);
    addChild(_resultOverlay, 220);

    cocos2d::LayerColor* shade = cocos2d::LayerColor::create(cocos2d::Color4B(5, 9, 22, 198), kDesignSize.width, kDesignSize.height);
    _resultOverlay->addChild(shade);

    cocos2d::LayerColor* resultPanel =
        cocos2d::LayerColor::create(cocos2d::Color4B(13, 28, 61, 238), 640.0f, 460.0f);
    resultPanel->setPosition(cocos2d::Vec2(220.0f, 760.0f));
    resultPanel->setTag(301);
    _resultOverlay->addChild(resultPanel);

    _resultTitleLabel = cocos2d::Label::createWithSystemFont("Well Done!", "Arial", 52);
    _resultTitleLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_TOP);
    _resultTitleLabel->setPosition(cocos2d::Vec2(320.0f, 390.0f));
    _resultTitleLabel->setTextColor(cocos2d::Color4B(255, 238, 184, 255));
    resultPanel->addChild(_resultTitleLabel);

    _resultStarsLabel = cocos2d::Label::createWithSystemFont("* * *", "Arial", 56);
    _resultStarsLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _resultStarsLabel->setPosition(cocos2d::Vec2(320.0f, 252.0f));
    _resultStarsLabel->setTextColor(cocos2d::Color4B(255, 215, 92, 255));
    resultPanel->addChild(_resultStarsLabel);

    _resultBodyLabel = cocos2d::Label::createWithSystemFont("All cards cleared.", "Arial", 30);
    _resultBodyLabel->setDimensions(520.0f, 120.0f);
    _resultBodyLabel->setAlignment(cocos2d::TextHAlignment::CENTER);
    _resultBodyLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _resultBodyLabel->setPosition(cocos2d::Vec2(320.0f, 176.0f));
    _resultBodyLabel->setTextColor(cocos2d::Color4B(223, 238, 255, 235));
    resultPanel->addChild(_resultBodyLabel);

    cocos2d::LayerColor* restartButton =
        cocos2d::LayerColor::create(cocos2d::Color4B(246, 191, 87, 255), 226.0f, 82.0f);
    restartButton->setPosition(cocos2d::Vec2(207.0f, 52.0f));
    resultPanel->addChild(restartButton);

    cocos2d::Label* restartLabel = cocos2d::Label::createWithSystemFont("RESTART", "Arial", 30);
    restartLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    restartLabel->setPosition(cocos2d::Vec2(320.0f, 93.0f));
    restartLabel->setTextColor(cocos2d::Color4B(70, 40, 17, 255));
    resultPanel->addChild(restartLabel);

    bindTapToNode(restartButton, [this]() {
        if (_onRestartClick) {
            _onRestartClick();
        }
    });
}

void GameView::bindTapToNode(cocos2d::Node* targetNode, const std::function<void()>& onTap) {
    cocos2d::EventListenerTouchOneByOne* listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [targetNode](cocos2d::Touch* touch, cocos2d::Event*) {
        if (targetNode == nullptr || targetNode->getParent() == nullptr || !targetNode->isVisible()) {
            return false;
        }

        const cocos2d::Vec2 pointInParent =
            targetNode->getParent()->convertToNodeSpace(touch->getLocation());
        return targetNode->getBoundingBox().containsPoint(pointInParent);
    };
    listener->onTouchEnded = [targetNode, onTap](cocos2d::Touch* touch, cocos2d::Event*) {
        if (targetNode == nullptr || targetNode->getParent() == nullptr || !targetNode->isVisible()) {
            return;
        }

        const cocos2d::Vec2 pointInParent =
            targetNode->getParent()->convertToNodeSpace(touch->getLocation());
        if (targetNode->getBoundingBox().containsPoint(pointInParent) && onTap) {
            onTap();
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, targetNode);
}

CardView* GameView::ensureCardView(const CardModel& cardModel) {
    std::unordered_map<int, CardView*>::iterator iterator = _cardViews.find(cardModel.cardId);
    if (iterator != _cardViews.end()) {
        return iterator->second;
    }

    CardView* cardView = CardView::create(cardModel);
    if (cardView == nullptr) {
        return nullptr;
    }

    cardView->setTapCallback([this](int cardId) {
        if (_isInteractionEnabled && _onPlayfieldCardClick && !_isResultVisible) {
            _onPlayfieldCardClick(cardId);
        }
    });
    _cardLayer->addChild(cardView);
    _cardViews[cardModel.cardId] = cardView;
    return cardView;
}

void GameView::updateDrawPileDecorations(const GameModel& gameModel) {
    const int remainingDrawCount = static_cast<int>(gameModel.drawPileCardIds.size());
    const int remainingPlayfieldCount = getRemainingPlayfieldCount(gameModel);
    int totalPlayfieldCount = 0;
    for (size_t index = 0; index < gameModel.cards.size(); ++index) {
        if (gameModel.cards[index].initialPosition != cocos2d::Vec2::ZERO) {
            ++totalPlayfieldCount;
        }
    }

    if (_drawCountLabel != nullptr) {
        _drawCountLabel->setString(
            cocos2d::StringUtils::format("Remaining: %d", remainingDrawCount));
    }

    if (_statusLabel != nullptr) {
        _statusLabel->setString(
            cocos2d::StringUtils::format("Draw: %d  |  Live cards: %d", remainingDrawCount, remainingPlayfieldCount));
    }

    if (_progressLabel != nullptr) {
        _progressLabel->setString(
            cocos2d::StringUtils::format("Progress: %d / %d",
                totalPlayfieldCount - remainingPlayfieldCount,
                totalPlayfieldCount));
    }

    if (_drawPileTapProxy != nullptr) {
        _drawPileTapProxy->setVisible(!gameModel.drawPileCardIds.empty());
    }
}

void GameView::updateUndoButtonVisual() {
    if (_undoButton != nullptr) {
        const cocos2d::Color3B buttonColor = _isUndoEnabled
            ? cocos2d::Color3B(255, 209, 92)
            : cocos2d::Color3B(130, 122, 112);
        _undoButton->setColor(buttonColor);
    }

    if (_undoLabel != nullptr) {
        _undoLabel->setTextColor(_isUndoEnabled
            ? cocos2d::Color4B(75, 42, 18, 255)
            : cocos2d::Color4B(70, 70, 70, 255));
    }
}

void GameView::playEntranceAnimation(const GameModel& gameModel) {
    for (size_t index = 0; index < gameModel.cards.size(); ++index) {
        const CardModel& cardModel = gameModel.cards[index];
        CardView* cardView = ensureCardView(cardModel);
        if (cardView == nullptr || !cardView->isVisible()) {
            continue;
        }

        cocos2d::Vec2 targetPosition;
        float targetScale = 1.0f;
        bool showFaceUp = false;
        bool isInteractive = false;
        int zOrder = 0;
        if (!getCardPresentation(
                gameModel,
                cardModel.cardId,
                &targetPosition,
                &targetScale,
                &showFaceUp,
                &isInteractive,
                &zOrder)) {
            continue;
        }

        const float delay = static_cast<float>(index) * 0.035f;
        const cocos2d::Vec2 startPosition = cardModel.isPlayfieldCard()
            ? targetPosition + cocos2d::Vec2(0.0f, 220.0f)
            : targetPosition + cocos2d::Vec2(0.0f, -110.0f);

        cardView->stopAllActions();
        cardView->setOpacity(0);
        cardView->setPosition(startPosition);
        cardView->setScale(targetScale * 0.74f);
        cardView->runAction(cocos2d::Sequence::create(
            cocos2d::DelayTime::create(delay),
            cocos2d::Spawn::create(
                cocos2d::FadeTo::create(0.18f, 255),
                cocos2d::EaseBackOut::create(cocos2d::MoveTo::create(0.25f, targetPosition)),
                cocos2d::EaseBackOut::create(cocos2d::ScaleTo::create(0.25f, targetScale)),
                nullptr),
            nullptr));
    }
}

int GameView::getRemainingPlayfieldCount(const GameModel& gameModel) const {
    int remainingCount = 0;
    for (size_t index = 0; index < gameModel.cards.size(); ++index) {
        if (gameModel.cards[index].isPlayfieldCard()) {
            ++remainingCount;
        }
    }
    return remainingCount;
}

}  // namespace cardgame
