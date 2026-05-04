#ifndef CARD_MATCH_GAME_CARD_TYPES_H
#define CARD_MATCH_GAME_CARD_TYPES_H

namespace cardgame {

// 花色类型。
enum CardSuitType {
    CST_NONE = -1,
    CST_CLUBS = 0,
    CST_DIAMONDS,
    CST_HEARTS,
    CST_SPADES,
    CST_NUM_CARD_SUIT_TYPES
};

// 点数类型。
enum CardFaceType {
    CFT_NONE = -1,
    CFT_ACE = 0,
    CFT_TWO,
    CFT_THREE,
    CFT_FOUR,
    CFT_FIVE,
    CFT_SIX,
    CFT_SEVEN,
    CFT_EIGHT,
    CFT_NINE,
    CFT_TEN,
    CFT_JACK,
    CFT_QUEEN,
    CFT_KING,
    CFT_NUM_CARD_FACE_TYPES
};

// 运行时所在区域。
enum CardAreaType {
    CAT_NONE = -1,
    CAT_PLAYFIELD = 0,
    CAT_DRAW_PILE,
    CAT_TRAY
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_CARD_TYPES_H
