#ifndef CARD_MATCH_GAME_GAME_MODEL_FROM_LEVEL_GENERATOR_H
#define CARD_MATCH_GAME_GAME_MODEL_FROM_LEVEL_GENERATOR_H

#include <algorithm>
#include <random>
#include <vector>

#include "configs/models/CardResConfig.h"
#include "configs/models/LevelConfig.h"
#include "models/GameModel.h"

namespace cardgame {

/**
 * 关卡到运行时模型的转换服务。
 * 负责根据静态配置生成卡牌 ID、区域归属以及主牌区遮挡关系。
 */
class GameModelFromLevelGenerator {
public:
    /**
     * 生成一局新的游戏运行时模型。
     * @param levelConfig 静态关卡配置。
     * @param outGameModel 输出的运行时模型。
     * @return 生成是否成功。
     */
    static bool generateGameModel(const LevelConfig& levelConfig, GameModel* outGameModel) {
        if (outGameModel == nullptr || !levelConfig.isValid()) {
            return false;
        }

        GameModel gameModel;
        int nextCardId = 1;

        for (size_t index = 0; index < levelConfig.playfieldCards.size(); ++index) {
            const LevelCardConfig& config = levelConfig.playfieldCards[index];
            CardModel cardModel;
            cardModel.cardId = nextCardId++;
            cardModel.cardFace = config.cardFace;
            cardModel.cardSuit = config.cardSuit;
            cardModel.areaType = CAT_PLAYFIELD;
            cardModel.position = config.position;
            cardModel.initialPosition = config.position;
            gameModel.cards.push_back(cardModel);
        }

        for (size_t index = 0; index < levelConfig.stackCards.size(); ++index) {
            const LevelCardConfig& config = levelConfig.stackCards[index];
            CardModel cardModel;
            cardModel.cardId = nextCardId++;
            cardModel.cardFace = config.cardFace;
            cardModel.cardSuit = config.cardSuit;
            cardModel.areaType = index == 0 ? CAT_TRAY : CAT_DRAW_PILE;
            cardModel.position = config.position;
            cardModel.initialPosition = config.position;
            cardModel.isFaceUp = true;
            gameModel.cards.push_back(cardModel);

            if (index == 0) {
                gameModel.trayHistoryCardIds.push_back(cardModel.cardId);
            } else {
                gameModel.drawPileCardIds.push_back(cardModel.cardId);
            }
        }

        if (gameModel.drawPileCardIds.size() > 1) {
            std::random_device randomDevice;
            std::mt19937 randomEngine(randomDevice());
            std::shuffle(gameModel.drawPileCardIds.begin(), gameModel.drawPileCardIds.end(), randomEngine);
        }

        buildCoverRelations(&gameModel);
        gameModel.refreshPlayfieldFaceUpState();
        *outGameModel = gameModel;
        return true;
    }

private:
    static void buildCoverRelations(GameModel* gameModel) {
        if (gameModel == nullptr) {
            return;
        }

        const cocos2d::Size cardSize = CardResConfig::getPlayfieldDisplaySize();
        for (size_t lowerIndex = 0; lowerIndex < gameModel->cards.size(); ++lowerIndex) {
            CardModel& lowerCard = gameModel->cards[lowerIndex];
            if (!lowerCard.isPlayfieldCard()) {
                continue;
            }

            lowerCard.coveredByCardIds.clear();
            const cocos2d::Rect lowerRect(
                lowerCard.position.x,
                lowerCard.position.y,
                cardSize.width,
                cardSize.height);

            for (size_t upperIndex = 0; upperIndex < gameModel->cards.size(); ++upperIndex) {
                if (lowerIndex == upperIndex) {
                    continue;
                }

                const CardModel& upperCard = gameModel->cards[upperIndex];
                if (!upperCard.isPlayfieldCard()) {
                    continue;
                }
                if (upperCard.position.y <= lowerCard.position.y + 8.0f) {
                    continue;
                }

                const cocos2d::Rect upperRect(
                    upperCard.position.x,
                    upperCard.position.y,
                    cardSize.width,
                    cardSize.height);
                if (!upperRect.intersectsRect(lowerRect)) {
                    continue;
                }

                const float overlapLeft = std::max(lowerRect.getMinX(), upperRect.getMinX());
                const float overlapRight = std::min(lowerRect.getMaxX(), upperRect.getMaxX());
                const float overlapWidth = overlapRight - overlapLeft;
                if (overlapWidth >= cardSize.width * 0.4f) {
                    lowerCard.coveredByCardIds.push_back(upperCard.cardId);
                }
            }
        }
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_GAME_MODEL_FROM_LEVEL_GENERATOR_H
