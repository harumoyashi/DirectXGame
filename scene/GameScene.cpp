#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>

using namespace DirectX;

GameScene::GameScene() {}

GameScene::~GameScene() { delete model_; }

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	//ファイルを指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("mario.jpg");

	// 3Dモデルの生成
	model_ = Model::Create();

	for (size_t i = 0; i < _countof(worldTransform_); i++) {
		for (size_t j = 0; j < _countof(worldTransform_); j++) {
			for (size_t k = 0; k < _countof(worldTransform_); k++) {
				// x,y,z方向のスケーリングを設定
				worldTransform_[k][j][i].scale_ = {1.0f, 1.0f, 1.0f};

				// x,y,z軸周りの回転角を設定
				worldTransform_[k][j][i].rotation_ = {0.0f, 0.0f, 0.0f};

				// x,y,z軸周りの平行移動を設定
				worldTransform_[k][j][i].translation_ = {
				  i * 4.0f - 15.0f, j * 4.0f - 15.0f, k * 2.0f};

				//ワールドトランスフォームの初期化
				worldTransform_[k][j][i].Initialize();
			}
		}
	}

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();
}

void GameScene::Update() {}

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
		for (size_t j = 0; j < _countof(worldTransform_); j++) {
			for (size_t k = 0; k < _countof(worldTransform_); k++) {
				model_->Draw(worldTransform_[k][j][i], viewProjection_, textureHandle_);
			}
		}
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
