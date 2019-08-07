#include <Siv3D.hpp> // OpenSiv3D v0.3.2
#include <string>
#include <random>
#include <array>
#include <cmath>
#define debug(var) std::cout << #var << " : " << var << std::endl;
#define radian(angle) angle * (pi) / 180

//定数宣言
constexpr int WINDOW_X = 1280;
constexpr int WINDOW_Y = 720;
constexpr int FIELD_X = 960;
constexpr int FIELD_Y = 720;
constexpr int OBJECT_WIDTH = 64;
constexpr int OBJECT_HEIGHT = 64;
constexpr int SPEED = 12;
constexpr int P_BULLET_NUM = 8;
constexpr int INVALID_NUM = -1;
constexpr int P_BULLET_SIZE = 16;
constexpr int P_BULLET_SPEED = 24;
constexpr int ENEMY_NUM = 16;
constexpr int E_PATTERN_NUM = 3;
constexpr int ENEMY_SIZE = 32;
constexpr int ENEMY_SPEED = 8;
constexpr int ENEMY_DAMAGE = 20;
constexpr int MY_HP = 1000;
constexpr double pi = 3.141592;
constexpr int TITLE_SIZE = 32; //メニューテキストの縦の長さ
constexpr int MENU_NUM = 2;

struct P_bullet {
  bool flag;
  double x[3], y[3];//[0] -> センター [1] -> 左 [2] -> 右
  int angle; //度数法
  P_bullet() {
	flag = false;
	angle = 45;
	for (int i = 0; i < 3; i++) {
	  x[i] = INVALID_NUM;
	  y[i] = INVALID_NUM;
	}
  }
};

struct Player {
  double x;
  double y;
  int hp;
  Texture img;
  Texture bullet_img;
  P_bullet bullets[P_BULLET_NUM];//3方向に飛ばすので
  Player(int x, int y, int hp, std::u32string img, std::u32string bullet_img) : x(x), y(y), hp(hp), img(img), bullet_img(bullet_img) {}
};

struct Enemy {
  double x, y;
  int angle;//向かう方向の角度
  bool isalive;
};

bool inField(int x, int y, int size) {//フィールド内にいるかどうかを返す関数
  return (0 <= x && x + size <= FIELD_X && 0 <= y && y + size <= FIELD_Y);
}

void enemyInitialize(Enemy & e, std::mt19937 & mt, std::uniform_int_distribution<int> & distx, std::uniform_int_distribution<int> & disty, std::uniform_int_distribution<int> & distangle) {
  e.x = distx(mt);
  e.y = disty(mt);
  e.angle = distangle(mt);
  e.isalive = true;
}//敵インスタンスの初期化

void Main() {

  //ゲーム関連の設定
  Window::Resize(WINDOW_X, WINDOW_Y);
  Graphics::SetBackground(Palette::Skyblue);
  Window::SetTitle(U"THE SHOOT.ver=2");
  Font font(20);//デバッグ用
  Font title(TITLE_SIZE);//メニューテキスト用
  Player player((FIELD_X - OBJECT_WIDTH) / 2, FIELD_Y - OBJECT_HEIGHT, MY_HP, U"img/player.png", U"img/bullet.png");
  Enemy enemies[ENEMY_NUM];
  /*敵の位置等の生成用の変数*/
  std::mt19937 mt(std::random_device{}());
  std::uniform_int_distribution<int> distx(0, FIELD_X - ENEMY_SIZE);
  std::uniform_int_distribution<int> disty(0, (FIELD_Y - ENEMY_SIZE) / 2);
  std::uniform_int_distribution<int> distangle(0, 180);
  for (Enemy& e : enemies) {
	enemyInitialize(e, mt, distx, disty, distangle);
  }//敵のインスタンスをそれぞれ初期化

  Texture background(U"img/background.png");
  int scrollVal = FIELD_Y - 2160;//背景のスクロール値
  int count = 0;//今何フレーム目か
  int gameType = 0;//0 -> ゲーム選択画面
  std::array<std::u32string, MENU_NUM> menuTexts = { U"ステージ1", U"対戦プレイ" };

  while (System::Update()) {//メインループ

	count++;

	if (gameType == 0) {
	  for (int i = 1; i <= menuTexts.size(); i++) {
		int y = i * TITLE_SIZE + 300;
		title(menuTexts[i - 1]).drawAt(WINDOW_X / 2, y);
	  }
	}

	if (gameType == 1) {

	  {
		background.draw(0, scrollVal);
		scrollVal++;
		if (scrollVal == 0)scrollVal = FIELD_Y - 2160;
	  }//背景の描画と移動

	  {
		player.img.draw(player.x, player.y);//Draw player.

		if (KeyRight.pressed()) {
		  player.x += SPEED;
		}
		else if (KeyLeft.pressed()) {
		  player.x -= SPEED;
		}
		else if (KeyUp.pressed()) {
		  player.y -= SPEED;
		}
		else if (KeyDown.pressed()) {
		  player.y += SPEED;
		}
		if (KeyEscape.down()) {
		  break;
		}
		if (player.x < 0)player.x = 0;
		else if (player.x > FIELD_X - OBJECT_WIDTH)player.x = FIELD_X - OBJECT_WIDTH;
		else if (player.y < 0)player.y = 0;
		else if (player.y > FIELD_Y - OBJECT_HEIGHT)player.y = FIELD_Y - OBJECT_HEIGHT;

	  }//自機の描画と移動

	  {

		if (KeySpace.pressed() && count % 6 == 0) {
		  for (P_bullet& bullet : player.bullets) {
			if (bullet.flag == false) {
			  bullet.flag = true;
			  for (int i = 0; i < 3; i++) {
				bullet.x[i] = player.x + (OBJECT_WIDTH - P_BULLET_SIZE) / 2;
				bullet.y[i] = player.y - P_BULLET_SIZE;
			  }
			  break;
			}
		  }
		}

		for (P_bullet& bullet : player.bullets) {
		  if (bullet.flag) {
			for (int i = 0; i < 3; i++) {
			  player.bullet_img.draw(bullet.x[i], bullet.y[i]);
			  switch (i) {
			  case 0:
				bullet.y[i] -= P_BULLET_SPEED;
				break;
			  case 1:
			  case 2:
				double moveY = sin(radian(bullet.angle)) * P_BULLET_SPEED;
				double moveX = cos(radian(bullet.angle)) * P_BULLET_SPEED;
				if (i == 1) {
				  bullet.x[i] -= moveX;
				  bullet.y[i] -= moveY;
				}
				if (i == 2) {
				  bullet.x[i] += moveX;
				  bullet.y[i] -= moveY;
				}
				break;
			  }
			  if (!inField(bullet.x[i], bullet.y[i], P_BULLET_SIZE)) {
				bullet.flag = false;
			  }
			}
		  }
		}

	  }//自機の弾の処理

	  {
		for (Enemy& e : enemies) {
		  if (e.isalive == true) {
			const auto enemyRect = Rect(e.x, e.y, ENEMY_SIZE, ENEMY_SIZE);
			const bool isTouched = enemyRect.intersects(player.img.region().movedBy(player.x, player.y));
			if (isTouched)player.hp -= ENEMY_DAMAGE;
			font(player.hp).draw();
			enemyRect.draw(isTouched ? Palette::Red : Palette::White);
			double moveY = sin(radian(e.angle)) * ENEMY_SPEED;
			double moveX = cos(radian(e.angle)) * ENEMY_SPEED;
			e.x -= moveX;
			e.y += moveY;
		  }
		  if (!inField(e.x, e.y, ENEMY_SIZE))e.isalive = false;
		}
		if (count % 2 == 0) {//8フレームごとに敵を復活させる
		  int tmpIndex = count % ENEMY_NUM;
		  if (!enemies[tmpIndex].isalive)enemyInitialize(enemies[tmpIndex], mt, distx, disty, distangle);
		}
	  }//敵の処理と当たり判定

	  {
		Rect(FIELD_X, 0, WINDOW_X, 80).draw(Palette::White);
		Rect(FIELD_X, 0, ((WINDOW_X - FIELD_X) * player.hp / MY_HP), 80).draw(Palette::Red);
	  }//HP欄を作る


	  if (player.hp <= 0)break;//HPが0になったら終了

	}//end of if.

  }//end of main loop.

}//end of entry point.