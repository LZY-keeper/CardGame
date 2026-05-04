#ifndef CARD_MATCH_GAME_GAME_CONTROLLER_H
#define CARD_MATCH_GAME_GAME_CONTROLLER_H

#include <vector>
#include <string>

#include "managers/UndoManager.h"
#include "models/GameModel.h"

namespace cardgame {

class GameView;

/**
 * 整局游戏主控制器。
 * 负责关卡启动、模型变更、匹配规则判断、抽牌逻辑与撤销流程协调。
 */
class GameController {
public:
    GameController();
    ~GameController();

    /**
     * 初始化控制器并启动首关。
     * @param gameView 当前绑定的主视图。
     * @param levelConfigPath 首关配置路径。
     * @return 初始化是否成功。
     */
    bool init(GameView* gameView, const std::string& levelConfigPath);

private:
    void bindViewCallbacks();
    void restartLevel();
    void finalizeMutation(const std::vector<int>& revealedCardIds);
    void handlePlayfieldCardClick(int cardId);
    void handleDrawPileClick();
    void handleUndoClick();
    void handleRestartClick();
    std::vector<int> collectNewlyFaceUpCardIds(const GameModel& previousModel) const;
    bool hasAnyPlayableCards() const;
    bool isVictory() const;
    bool isDefeat() const;
    int calculateVictoryStars() const;

    GameView* _gameView;
    GameModel _gameModel;
    UndoManager _undoManager;
    std::string _levelConfigPath;
    bool _isAnimating;
    bool _isGameFinished;
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_GAME_CONTROLLER_H
