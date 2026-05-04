#ifndef CARD_MATCH_GAME_UNDO_MANAGER_H
#define CARD_MATCH_GAME_UNDO_MANAGER_H

#include <vector>

#include "models/UndoModel.h"

namespace cardgame {

/**
 * 回退记录管理器。
 * 作为控制器成员使用，负责压栈、出栈与重置撤销历史。
 */
class UndoManager {
public:
    /** 清空所有回退记录。 */
    void clear() {
        _records.clear();
    }

    /**
     * 追加一条回退记录。
     * @param undoModel 需要压栈的回退数据。
     */
    void pushRecord(const UndoModel& undoModel) {
        _records.push_back(undoModel);
    }

    /** 返回当前是否存在可回退记录。 */
    bool canUndo() const {
        return !_records.empty();
    }

    /**
     * 弹出最后一条回退记录。
     * @return 最近一次回退数据；若不存在则返回默认对象。
     */
    UndoModel popRecord() {
        if (_records.empty()) {
            return UndoModel();
        }

        const UndoModel result = _records.back();
        _records.pop_back();
        return result;
    }

private:
    /** 回退记录栈。 */
    std::vector<UndoModel> _records;
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_UNDO_MANAGER_H
