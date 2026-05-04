#ifndef CARD_MATCH_GAME_CARD_RULE_SERVICE_H
#define CARD_MATCH_GAME_CARD_RULE_SERVICE_H

#include <cstdlib>

#include "models/CardTypes.h"

namespace cardgame {

/**
 * 卡牌规则服务。
 * 负责提供无状态的点数比较与匹配规则判断。
 */
class CardRuleService {
public:
    /**
     * 判断两张牌是否满足相差 1 的匹配规则。
     * @param sourceFace 被点击卡牌点数。
     * @param trayFace 当前底牌点数。
     * @return 是否可匹配。
     */
    static bool canMatch(CardFaceType sourceFace, CardFaceType trayFace) {
        if (sourceFace == CFT_NONE || trayFace == CFT_NONE) {
            return false;
        }
        return std::abs(toOrdinal(sourceFace) - toOrdinal(trayFace)) == 1;
    }

private:
    static int toOrdinal(CardFaceType cardFace) {
        return static_cast<int>(cardFace);
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_CARD_RULE_SERVICE_H
