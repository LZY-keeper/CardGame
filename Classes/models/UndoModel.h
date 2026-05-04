#ifndef CARD_MATCH_GAME_UNDO_MODEL_H
#define CARD_MATCH_GAME_UNDO_MODEL_H

#include "models/GameModel.h"

namespace cardgame {

// 回退动作类型。
enum UndoActionType {
    UAT_NONE = 0,
    UAT_DRAW_FROM_STACK,
    UAT_MATCH_PLAYFIELD
};

/**
 * 单条回退记录。
 * 负责保存触发回退前的整局快照以及本次动作的关键卡牌。
 */
class UndoModel {
public:
    /** 回退动作类型。 */
    UndoActionType actionType;
    /** 本次动作中移动的卡牌 ID。 */
    int movedCardId;
    /** 回退前的完整游戏快照。 */
    GameModel snapshot;

    UndoModel()
        : actionType(UAT_NONE)
        , movedCardId(-1) {
    }

    /**
     * 构造一条完整回退记录。
     * @param inActionType 回退动作类型。
     * @param inMovedCardId 本次动作中被移动的卡牌 ID。
     * @param inSnapshot 动作执行前的游戏快照。
     */
    UndoModel(UndoActionType inActionType, int inMovedCardId, const GameModel& inSnapshot)
        : actionType(inActionType)
        , movedCardId(inMovedCardId)
        , snapshot(inSnapshot) {
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_UNDO_MODEL_H
