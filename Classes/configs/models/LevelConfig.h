#ifndef CARD_MATCH_GAME_LEVEL_CONFIG_H
#define CARD_MATCH_GAME_LEVEL_CONFIG_H

#include <vector>

#include "cocos2d.h"
#include "models/CardTypes.h"

namespace cardgame {

/**
 * 关卡中的单张卡牌静态配置。
 * 用于描述卡牌初始点数、花色以及在桌面上的初始摆放位置。
 */
struct LevelCardConfig {
    /** 卡牌点数。 */
    CardFaceType cardFace;
    /** 卡牌花色。 */
    CardSuitType cardSuit;
    /** 卡牌左下角位置。 */
    cocos2d::Vec2 position;

    LevelCardConfig()
        : cardFace(CFT_NONE)
        , cardSuit(CST_NONE)
        , position(cocos2d::Vec2::ZERO) {
    }
};

/**
 * 关卡静态配置。
 * 负责存储主牌区与备用牌堆的初始摆放数据。
 */
class LevelConfig {
public:
    /** 主牌区卡牌配置。 */
    std::vector<LevelCardConfig> playfieldCards;
    /** 备用牌堆配置，首张牌作为初始底牌。 */
    std::vector<LevelCardConfig> stackCards;

    /** 返回当前关卡配置是否具备最基本的开局条件。 */
    bool isValid() const {
        return !playfieldCards.empty() && !stackCards.empty();
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_LEVEL_CONFIG_H
