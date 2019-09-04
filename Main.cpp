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
#include "Header.hpp"
#include "gameconnect.hpp"

#define radian(angle) (angle) * (pi) / 180
#define PAD_UP (Gamepad(0).isConnected() ? pad.povUp.pressed() : KeyUp.pressed())
#define PAD_DOWN (Gamepad(0).isConnected() ? pad.povDown.pressed() : KeyDown.pressed())
#define PAD_RIGHT (Gamepad(0).isConnected() ? pad.povRight.pressed() : KeyRight.pressed())
#define PAD_LEFT (Gamepad(0).isConnected() ? pad.povLeft.pressed() : KeyLeft.pressed())
#define PAD_ENTER (Gamepad(0).isConnected() ? pad.buttons[0].pressed() : KeyEnter.pressed())

bool inField(int x, int y, int width, int height) {//フィールド内にいるかどうかを返す関数
  return (0 <= x && x + width <= FIELD_X && 0 <= y && y + height <= FIELD_Y);
}

void Init() {//ウィンドウ関係の設定 
  Window::Resize(WINDOW_X, WINDOW_Y);
  Graphics::SetBackground(Palette::Skyblue);
  Window::SetTitle(U"THE SHOOT.ver=2");
  Console.open();
}

void Stage1::update(std::unique_ptr<Stage> & ptr) {

  try {

	if (Gamepad(0).isConnected()) {
	  pad = Gamepad(0);//パッド情報を毎回更新する
	}
	count++;
	{
	  background.draw(0, scrollVal);
	  scrollVal += SCROLL_SPEED;
	  if (scrollVal >= 0)scrollVal = FIELD_Y - 2160;
	}//背景の描画と移動

	if (player.isAwakening) {
	  if (count % 4 == 0) {
		for (int i = 0; i < 9; i++) {
		  afterimages[i + 1] = afterimages[i];
		}//それぞれの値を一つずつずらす
		afterimages[0] = std::make_pair(player.x, player.y);
	  }
	  for (int i = 0; i < 10; i++) {
		player.img.draw(afterimages[i].first, afterimages[i].second, Color(0, 255, 255));
	  }
	  //残像の描画
	}//自機の残像の描画(自機が覚醒したら残像を描画する)

	{
	  player.img.draw(player.x, player.y);//Draw player.

	  if (PAD_RIGHT) {
		player.x += SPEED;
	  }
	  else if (PAD_LEFT) {
		player.x -= SPEED;
	  }
	  else if (PAD_UP) {
		player.y -= SPEED;
	  }
	  else if (PAD_DOWN) {
		player.y += SPEED;
	  }
	  if (KeyEscape.down()) {
		return;
	  }
	  if (player.x < 0)player.x = 0;
	  else if (player.x > FIELD_X - OBJECT_WIDTH)player.x = FIELD_X - OBJECT_WIDTH;
	  else if (player.y < 0)player.y = 0;
	  else if (player.y > FIELD_Y - OBJECT_HEIGHT)player.y = FIELD_Y - OBJECT_HEIGHT;

	}//自機の描画と移動

	{

	  if (PAD_ENTER && count % 6 == 0) {
		for (P_bullet& bullet : player.bullets) {
		  if (!bullet.flag[0] && !bullet.flag[1] && !bullet.flag[2]) {
			for (int i = 0; i < 3; i++) {
			  bullet.flag[i] = true;
			  bullet.x[i] = player.x + (OBJECT_WIDTH - P_BULLET_SIZE) / 2;
			  bullet.y[i] = player.y - P_BULLET_SIZE;
			}
			break;
		  }
		}
	  }

	  for (P_bullet& bullet : player.bullets) {
		for (int i = 0; i < 3; i++) {
		  if (bullet.flag[i]) {
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
			if (!inField(bullet.x[i], bullet.y[i], P_BULLET_SIZE, P_BULLET_SIZE)) {
			  bullet.flag[i] = false;
			}
		  }
		}
	  }

	}//自機の弾の処理


	{
	  for (Enemy& e : enemies) {
		if (e.isalive) {
		  if (e.istouched)e.hp -= P_ATTACK_DAMAGE;
		  if (e.y + ENEMY_HEIGHT >= FIELD_Y) {
			player.hp -= ENEMY_DAMAGE;
			Rect(0, FIELD_Y - 10, FIELD_X, 10).draw(Palette::Red);
		  }
		  if (e.hp <= 0)e.isalive = false;
		  Rect(e.x, e.y - 20, (ENEMY_WIDTH * e.hp / ENEMY_HP), 20).draw(Palette::Lightgreen);
		  const auto enemyRect = Rect(e.x, e.y, ENEMY_WIDTH, ENEMY_HEIGHT);
		  const bool isTouched = enemyRect.intersects(player.img.region().movedBy(player.x, player.y));
		  if (e.istouched)attacked_enemy_img.rotatedAt(ENEMY_WIDTH / 2, ENEMY_HEIGHT / 2, -radian(e.angle - 90)).draw(e.x, e.y);
		  else enemy_img.rotatedAt(ENEMY_WIDTH / 2, ENEMY_HEIGHT / 2, -radian(e.angle - 90)).draw(e.x, e.y);
		  double moveY = sin(radian(e.angle)) * ENEMY_SPEED;
		  double moveX = cos(radian(e.angle)) * ENEMY_SPEED;
		  e.x -= moveX;
		  e.y += moveY;
		}
		if (!inField(e.x, e.y, ENEMY_WIDTH, ENEMY_HEIGHT))e.isalive = false;
	  }

	  if (count % 2 == 0) {//2フレームごとに敵を復活させる
		int tmpIndex = count % ENEMY_NUM;
		if (!enemies[tmpIndex].isalive)enemyInitialize(enemies[tmpIndex], mt, distx, distangle);
	  }
	}//敵の処理

	{
	  for (auto& b : player.bullets) {
		for (int i = 0; i < 3; i++) {
		  for (auto& e : enemies) {
			if (e.isalive && Rect(b.x[i], b.y[i], P_BULLET_SIZE, P_BULLET_SIZE).intersects(Rect(e.x, e.y, ENEMY_WIDTH, ENEMY_HEIGHT)))e.istouched = true;
		  }
		}
	  }
	  for (auto& e : enemies) {
		bool flag = false;
		for (auto& b : player.bullets) {
		  for (int i = 0; i < 3; i++) {
			if (e.isalive && Rect(b.x[i], b.y[i], P_BULLET_SIZE, P_BULLET_SIZE).intersects(Rect(e.x, e.y, ENEMY_WIDTH, ENEMY_HEIGHT)))flag = true;
		  }
		}
		e.istouched = flag;
	  }
	}//敵と自機の弾の当たり判定

	{
	  Rect(FIELD_X, 0, WINDOW_X, 80).draw(Palette::White);
	  Rect(FIELD_X, 0, ((WINDOW_X - FIELD_X) * player.hp / MY_HP), 80).draw(Palette::Red);
	}//HP欄を作る

	if (player.hp <= 0) {
	  int x = 114514;
	  connection.Send(Jin::getIP(), 3000, x);
	  ptr = std::make_unique<Introduction>(std::initializer_list<String>{U"HPが0になりました！\n次の人に代わってください"});
	}

  }
  catch (std::exception & e) {
	std::cout << e.what() << std::endl;
  }

}

void Introduction::update(std::unique_ptr<Stage> & ptr) {
  for (const auto& s : sentences) {
	const Rect rect = font(s).region();//それぞれの文字列の縦と横の長さを取得
	int x = (WINDOW_X - rect.w) / 2, y = (WINDOW_Y - rect.h) / 2;//文字列の描画位置を設定
	for (int i = 0; i < s.size(); i++) {
	  String tmp = U"";
	  for (int j = 0; j < i; j++)tmp += s[j];
	  //描画する文字列の作成
	  font(tmp).draw(x, y);//描画
	  System::Update();//システムのアップデートする
	  std::this_thread::sleep_for(50ms);
	}
	while (!PAD_ENTER && System::Update())font(s).draw(x, y);
  }
  ptr = std::make_unique<Stage1>();
}

void Main() {

  Init();
  std::unique_ptr<Stage> stage_ptr = std::make_unique<Introduction>(std::initializer_list<String>{U"これからルール説明を始めるよ", U"うんち！"});

  while (System::Update()) {
	stage_ptr->update(stage_ptr);
  }

}//end of entry point.