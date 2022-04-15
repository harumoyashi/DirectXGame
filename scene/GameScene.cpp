#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>

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

	//乱数シード生成器
	std::random_device seed_gen;
	//メルセンヌ・ツイスター
	std::mt19937_64 engine(seed_gen());
	//乱数範囲(回転角用)
	std::uniform_real_distribution<float> rotDist(0.0f, XM_2PI);
	//乱数範囲(座標用)
	std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);

	// x,y,z方向のスケーリングを設定
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// x,y,z軸周りの回転角を設定
	worldTransform_.rotation_ = {rotDist(engine), rotDist(engine), rotDist(engine)};

	// x,y,z軸周りの平行移動を設定
	worldTransform_.translation_ = {0, 0, 0};

	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	for (size_t i = 0; i < _countof(viewProjection_); i++) {
		//カメラ視点座標を設定
		viewProjection_[i].eye = {posDist(engine), posDist(engine), posDist(engine)};

		//カメラ注視点座標を設定
		viewProjection_[i].target = {0, 0, 0};

		//カメラ上方向ベクトルを設定(右上45度指定)
		viewProjection_[i].up = {0.0f, 1.0f, 0.0f};

		//ビュープロジェクションの初期化
		viewProjection_[i].Initialize();
	}
}

void GameScene::Update() {
	////視点の移動処理
	////視点の移動ベクトル
	// XMFLOAT3 move = {0, 0, 0};

	////視点の移動速度
	// const float kEyeSpeed = 0.2f;

	////押した方向で移動ベクトルを変更
	// if (input_->PushKey(DIK_W)) {
	//	move = {0, 0, kEyeSpeed};
	// } else if (input_->PushKey(DIK_S)) {
	//	move = {0, 0, -kEyeSpeed};
	// }

	// for (size_t i = 0; i < _countof(viewProjection_); i++) {
	//	//視点移動（ベクトルの加算）
	//	viewProjection_[i].eye.x += move.x;
	//	viewProjection_[i].eye.y += move.y;
	//	viewProjection_[i].eye.z += move.z;

	//	//行列の再計算
	//	viewProjection_[i].UpdateMatrix();
	//}

	////注視点の移動処理
	////注視点の移動ベクトル
	// XMFLOAT3 tMove = {0, 0, 0};

	////注視点の移動速度
	// const float kTatgetSpeed = 0.2f;

	////押した方向で移動ベクトルを変更
	// if (input_->PushKey(DIK_LEFT)) {
	//	tMove = {-kTatgetSpeed, 0, 0};
	// } else if (input_->PushKey(DIK_RIGHT)) {
	//	tMove = {kTatgetSpeed, 0, 0};
	// }

	// for (size_t i = 0; i < _countof(viewProjection_); i++) {
	//	//注視点移動（ベクトルの加算）
	//	viewProjection_[i].target.x += tMove.x;
	//	viewProjection_[i].target.y += tMove.y;
	//	viewProjection_[i].target.z += tMove.z;

	//	//行列の再計算
	//	viewProjection_[i].UpdateMatrix();
	//}

	////上方向回転処理
	////上方向の回転速度[ラジアン/frame]
	// const float kUpRotSpeed = 0.05f;

	////押した方向で移動ベクトルを変更
	// if (input_->PushKey(DIK_SPACE)) {
	//	viewAngle += kUpRotSpeed;
	//	// 2πを超えたら0に戻す
	//	viewAngle = fmodf(viewAngle, XM_2PI);
	// }

	if (input_->TriggerKey(DIK_SPACE)) {
		if (viewNum < 2) {
			viewNum++;
		} else {
			viewNum = 0;
		}
	}

	for (size_t i = 0; i < _countof(viewProjection_); i++) {
		////上方向ベクトルを計算(半径1の円周上の座標)
		// viewProjection_[i].up = {cosf(viewAngle), sinf(viewAngle), 0.0f};

		////行列の再計算
		// viewProjection_[i].UpdateMatrix();

		//デバッグ用表示
		debugText_->SetPos(50, i * 70);
		debugText_->Printf(
		  "eye:(%f,%f,%f)", viewProjection_[i].eye.x, viewProjection_[i].eye.y,
		  viewProjection_[i].eye.z);

		debugText_->SetPos(50, i * 70 + 20);
		debugText_->Printf(
		  "target:(%f,%f,%f)", viewProjection_[i].target.x, viewProjection_[i].target.y,
		  viewProjection_[i].target.z);

		debugText_->SetPos(50, i * 70 + 40);
		debugText_->Printf(
		  "up:(%f,%f,%f)", viewProjection_[i].up.x, viewProjection_[i].up.y,
		  viewProjection_[i].up.z);
	}
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
	model_->Draw(worldTransform_, viewProjection_[viewNum], textureHandle_);
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
