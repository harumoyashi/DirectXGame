#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>

using namespace DirectX;

GameScene::GameScene() {}

GameScene::~GameScene() { 
	delete model_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	//ファイルを指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("mario.jpg");

	//3Dモデルの生成
	model_ = Model::Create();

	for (size_t i = 0; i < _countof(worldTransform_); i++) {
		// x,y,z方向のスケーリングを設定
		worldTransform_[i].scale_ = {1.0f, 1.0f, 1.0f};

		// x,y,z軸周りの回転角を設定
		worldTransform_[i].rotation_ = {0,0,0};

		// x,y,z軸周りの平行移動を設定
		worldTransform_[0].translation_ = {0.0f, 5.0f, 0.0f};
		worldTransform_[1].translation_ = {-5.0f, -2.5f, 0.0f};
		worldTransform_[2].translation_ = {5.0f, -2.5f, 0.0f};

		//ワールドトランスフォームの初期化
		worldTransform_[i].Initialize();
	}

	//カメラ視点座標を設定
	viewProjection_.eye = {0, 0, -25};

	//カメラ注視点座標を設定
	viewProjection_.target = worldTransform_[0].translation_;

	//カメラ上方向ベクトルを設定(右上45度指定)
	viewProjection_.up = {0, sinf(XM_PI / 2.0f), 0.0f};

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();
}

float GameScene::SetSpeed(float x, float nx) {
	tMoveFrame = 10; //ここ変えると移動仕切るまでのフレーム数が変わる
	return (x - nx) / tMoveFrame;
}

int GameScene::NumLoop(int x, int loopBreak) {
	if (x < loopBreak) {
		x++;
	} else {
		x = 0;
	}

	return x;
}

void GameScene::Update() {
	//注視点番号の切り替え
	if (isTargetChange == false) {
		if (input_->TriggerKey(DIK_SPACE)) {
			//注視点フラグオン
			isTargetChange = true;

			//注視点移動スピードを設定
			tSpeedX = SetSpeed(
			  worldTransform_[tViewNum].translation_.x, worldTransform_[tNextNum].translation_.x);

			tSpeedY = SetSpeed(
			  worldTransform_[tViewNum].translation_.y, worldTransform_[tNextNum].translation_.y);
		}
	}

	if (isTargetChange == true) {
		//注視点移動
		if (tMoveFrame > 0) {
			viewProjection_.target.x += tSpeedX;
			viewProjection_.target.y -= tSpeedY;

			tMoveFrame--;
		} 
		else {
			//次のビューに
			tViewNum = NumLoop(tViewNum, 2);
			tNextNum = NumLoop(tNextNum, 2);

			isTargetChange = false;
		}

		//行列の再計算
		viewProjection_.UpdateMatrix();
	}

	//デバッグ用表示
	debugText_->SetPos(50, 50);
	debugText_->Printf(
	  "eye:(%f,%f,%f)", viewProjection_.eye.x, viewProjection_.eye.y, viewProjection_.eye.z);

	debugText_->SetPos(50, 70);
	debugText_->Printf(
	  "target:(%f,%f,%f)", viewProjection_.target.x, viewProjection_.target.y,viewProjection_.target.z);

	debugText_->SetPos(50, 90);
	debugText_->Printf(
	  "up:(%f,%f,%f)", viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	// 3Dモデル描画
	for (size_t i = 0; i < _countof(worldTransform_); i++) {
		model_->Draw(worldTransform_[i], viewProjection_, textureHandle_);
	}
	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// デバッグテキストの描画
	debugText_->DrawAll(commandList);
	//
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
