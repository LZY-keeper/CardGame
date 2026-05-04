#ifndef CARD_MATCH_GAME_LEVEL_CONFIG_LOADER_H
#define CARD_MATCH_GAME_LEVEL_CONFIG_LOADER_H

#include <string>

#include "configs/models/LevelConfig.h"

namespace cardgame {

/**
 * 关卡配置加载器。
 * 负责从 JSON 文件中读取主牌区与备用牌堆的静态配置数据。
 */
class LevelConfigLoader {
public:
    /**
     * 加载指定关卡配置。
     * @param filePath Resources 相对路径。
     * @param outLevelConfig 输出的关卡配置对象。
     * @return 加载是否成功。
     */
    static bool loadLevelConfig(const std::string& filePath, LevelConfig* outLevelConfig);
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_LEVEL_CONFIG_LOADER_H
