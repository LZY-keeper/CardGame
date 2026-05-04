#ifndef CARD_MATCH_GAME_CARD_RES_CONFIG_H
#define CARD_MATCH_GAME_CARD_RES_CONFIG_H

#include <string>

#include "cocos2d.h"
#include "models/CardTypes.h"

namespace cardgame {

/**
 * 卡牌资源静态配置。
 * 统一管理卡牌底图、点数图、花色图以及运行时展示尺寸。
 */
class CardResConfig {
public:
    /** 返回卡牌正面的基础底图路径。 */
    static std::string getCardBasePath() {
        return "res/card_general.png";
    }

    /** 返回小号点数字图路径。 */
    static std::string getSmallNumberPath(CardFaceType cardFace, CardSuitType cardSuit) {
        return "res/number/small_" + getColorToken(cardSuit) + "_" + getFaceToken(cardFace) + ".png";
    }

    /** 返回大号点数字图路径。 */
    static std::string getBigNumberPath(CardFaceType cardFace, CardSuitType cardSuit) {
        return "res/number/big_" + getColorToken(cardSuit) + "_" + getFaceToken(cardFace) + ".png";
    }

    /** 返回花色图路径。 */
    static std::string getSuitPath(CardSuitType cardSuit) {
        switch (cardSuit) {
        case CST_CLUBS:
            return "res/suits/club.png";
        case CST_DIAMONDS:
            return "res/suits/diamond.png";
        case CST_HEARTS:
            return "res/suits/heart.png";
        case CST_SPADES:
            return "res/suits/spade.png";
        default:
            return std::string();
        }
    }

    /** 返回原始卡牌贴图尺寸。 */
    static cocos2d::Size getCardSourceSize() {
        return cocos2d::Size(182.0f, 282.0f);
    }

    /** 返回主牌区卡牌缩放。 */
    static float getPlayfieldScale() {
        return 0.78f;
    }

    /** 返回底牌区与备用牌堆卡牌缩放。 */
    static float getTrayScale() {
        return 0.82f;
    }

    /** 返回主牌区卡牌显示尺寸。 */
    static cocos2d::Size getPlayfieldDisplaySize() {
        const cocos2d::Size sourceSize = getCardSourceSize();
        return cocos2d::Size(
            sourceSize.width * getPlayfieldScale(),
            sourceSize.height * getPlayfieldScale());
    }

    /** 返回底牌区卡牌显示尺寸。 */
    static cocos2d::Size getTrayDisplaySize() {
        const cocos2d::Size sourceSize = getCardSourceSize();
        return cocos2d::Size(
            sourceSize.width * getTrayScale(),
            sourceSize.height * getTrayScale());
    }

    /** 返回牌背主色。 */
    static cocos2d::Color3B getBackTintColor() {
        return cocos2d::Color3B(56, 132, 235);
    }

private:
    static std::string getFaceToken(CardFaceType cardFace) {
        switch (cardFace) {
        case CFT_ACE:
            return "A";
        case CFT_TWO:
            return "2";
        case CFT_THREE:
            return "3";
        case CFT_FOUR:
            return "4";
        case CFT_FIVE:
            return "5";
        case CFT_SIX:
            return "6";
        case CFT_SEVEN:
            return "7";
        case CFT_EIGHT:
            return "8";
        case CFT_NINE:
            return "9";
        case CFT_TEN:
            return "10";
        case CFT_JACK:
            return "J";
        case CFT_QUEEN:
            return "Q";
        case CFT_KING:
            return "K";
        default:
            return "A";
        }
    }

    static std::string getColorToken(CardSuitType cardSuit) {
        return isRedSuit(cardSuit) ? "red" : "black";
    }

    static bool isRedSuit(CardSuitType cardSuit) {
        return cardSuit == CST_DIAMONDS || cardSuit == CST_HEARTS;
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_CARD_RES_CONFIG_H
