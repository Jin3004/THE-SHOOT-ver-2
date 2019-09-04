#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.3.2
#include <HamFramework.hpp>
#include <string>
#include <random>
#include <array>
#include <vector>
#include <cmath>
#include <thread>
#include <string_view>
#include <memory>
#include "gameconnect.hpp"

//定数宣言
#define WINDOW_X 1280
#define WINDOW_Y 720
#define FIELD_X 960
#define FIELD_Y 720
#define OBJECT_WIDTH 64
#define OBJECT_HEIGHT 64
#define SPEED 12
#define P_BULLET_NUM 8
#define INVALID_NUM -1
#define P_BULLET_SIZE 16
#define P_BULLET_SPEED 24
#define ENEMY_NUM 16
#define E_PATTERN_NUM 3
#define ENEMY_WIDTH 50
#define ENEMY_HEIGHT 100
#define ENEMY_SPEED 8
#define ENEMY_DAMAGE 100
#define MY_HP 1000
#define pi 3.141592
#define MENU_NUM 3
#define SCROLL_SPEED 8
#define ENEMY_HP 100
#define P_ATTACK_DAMAGE 10

struct P_bullet {
  bool flag[3] = { false,false,false };
  double x[3] = { INVALID_NUM, INVALID_NUM, INVALID_NUM }, y[3] = { INVALID_NUM, INVALID_NUM, INVALID_NUM };//[0] -> センター [1] -> 左 [2] -> 右
  int angle = 60; //度数法
};

struct Player {
  double x;
  double y;
  int hp;
  Texture img;
  Texture bullet_img;
  std::array<P_bullet, P_BULLET_NUM> bullets;
  bool isAwakening;
  Player(int x, int y, int hp, std::u32string img, std::u32string bullet_img) : x(x), y(y), hp(hp), img(img), bullet_img(bullet_img), isAwakening(false) {}
};

struct Enemy {
  double x, y;
  int angle;//向かう方向の角度
  bool isalive;
  bool istouched;
  int hp;
};

class Stage {
public:
  virtual void update(std::unique_ptr<Stage>&) = 0;
};

class Introduction : public Stage {
private:
  std::vector<String> sentences;
  Font font{ 50 };
  s3d::detail::Gamepad_impl pad = Gamepad(0);//ゲームパッド用の変数
public:
  Introduction(std::initializer_list<String> s) : sentences(s) {};
  void update(std::unique_ptr<Stage>&)override;
};

class Stage1 : public Stage {
private:
  void enemyInitialize(Enemy& e, std::mt19937& mt, std::uniform_int_distribution<int>& distx, std::uniform_int_distribution<int>& distangle) {
	e.x = distx(mt);
	e.y = 0;
	e.angle = distangle(mt);
	e.isalive = true;
	e.istouched = false;
	e.hp = ENEMY_HP;
  }//敵インスタンスの初期化

  Player player{ (FIELD_X - OBJECT_WIDTH) / 2, FIELD_Y - OBJECT_HEIGHT, MY_HP, U"img/player.png", U"img/bullet.png" };
  std::array<Enemy, ENEMY_NUM> enemies;
  std::mt19937 mt{ std::random_device{}() };
  std::uniform_int_distribution<int> distx{ 0, FIELD_X - ENEMY_WIDTH };
  std::uniform_int_distribution<int> distangle{ 60, 120 };
  int scrollVal = FIELD_Y - 2160;//背景のスクロール値
  std::array<std::pair<double, double>, 10> afterimages;
  Texture enemy_img{ U"img/enemy.png" };
  Texture attacked_enemy_img{ U"img/attacked_enemy.png" };
  Texture background{ U"img/background.png" };
  int count = 0;
  Font font{ 20 };
  s3d::detail::Gamepad_impl pad = Gamepad(0);//ゲームパッド用の変数
  Jin::GameConnect connection{ 3000 };

public:
  Stage1() {//初期化
	for (Enemy& e : enemies) {
	  enemyInitialize(e, mt, distx, distangle);
	}//敵のインスタンスをそれぞれ初期化
	std::cout << "Initialzed.";
  }
  void update(std::unique_ptr<Stage>& ptr)override;
};