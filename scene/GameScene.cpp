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

	////乱数シード生成器
	// std::random_device seed_gen;
	////メルセンヌ・ツイスター
	// std::mt19937_64 engine(seed_gen());
	////乱数範囲(回転角用)
	// std::uniform_real_distribution<float> rotDist(0.0f, XM_2PI);
	////乱数範囲(座標用)
	// std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);

	// for (size_t i = 0; i < _countof(worldTransform_); i++) {
	//	// x,y,z方向のスケーリングを設定
	//	worldTransform_[i].scale_ = {1.0f, 1.0f, 1.0f};

	//	// x,y,z軸周りの回転角を設定
	//	worldTransform_[i].rotation_ = {rotDist(engine), rotDist(engine), rotDist(engine)};

	//	// x,y,z軸周りの平行移動を設定
	//	worldTransform_[i].translation_ = {posDist(engine), posDist(engine), posDist(engine)};
	//}

	//ワールドトランスフォームの初期化
	//キャラクターの大元:親(0番)
	worldTransform_[PartId::Root].Initialize();
	//脊椎:子(1番)
	worldTransform_[PartId::Spine].translation_ = {0, 4.5f, 0};
	worldTransform_[PartId::Spine].parent_ = &worldTransform_[PartId::Root];
	worldTransform_[PartId::Spine].Initialize();

	//上半身
	//胸:子(2番)
	worldTransform_[PartId::Chest].translation_ = {0, 0, 0};
	worldTransform_[PartId::Chest].parent_ = &worldTransform_[PartId::Spine];
	worldTransform_[PartId::Chest].Initialize();
	//おっぱい:子(9番)
	worldTransform_[PartId::Oppai].translation_ = {0, 0, 2.5f};
	worldTransform_[PartId::Oppai].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::Oppai].Initialize();
	//頭:子(3番)
	worldTransform_[PartId::Head].translation_ = {0, 4.5f, 0};
	worldTransform_[PartId::Head].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::Head].Initialize();
	//左腕:子(4番)
	worldTransform_[PartId::ArmL].translation_ = {-4.5f, 0, 0};
	worldTransform_[PartId::ArmL].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::ArmL].Initialize();
	//右腕:子(5番)
	worldTransform_[PartId::ArmR].translation_ = {4.5f, 0, 0};
	worldTransform_[PartId::ArmR].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::ArmR].Initialize();

	//下半身
	//おしり:子(6番)
	worldTransform_[PartId::Hip].translation_ = {0, -4.5f, 0};
	worldTransform_[PartId::Hip].parent_ = &worldTransform_[PartId::Spine];
	worldTransform_[PartId::Hip].Initialize();
	//左足:子(7番)
	worldTransform_[PartId::LegL].translation_ = {-4.5f, -4.5f, 0};
	worldTransform_[PartId::LegL].parent_ = &worldTransform_[PartId::Hip];
	worldTransform_[PartId::LegL].Initialize();
	//右足:子(8番)
	worldTransform_[PartId::LegR].translation_ = {4.5f, -4.5f, 0};
	worldTransform_[PartId::LegR].parent_ = &worldTransform_[PartId::Hip];
	worldTransform_[PartId::LegR].Initialize();

	//カメラ視点座標を設定
	viewProjection_.eye = {0, 40.0f, -50.0f};

	//カメラ注視点座標を設定
	viewProjection_.target = {0, 0, 0};

	//カメラ上方向ベクトルを設定(右上45度指定)
	viewProjection_.up = {0.0f, sinf(XM_PI / 4.0f), 0.0f};

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();
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

	////視点移動（ベクトルの加算）
	// viewProjection_.eye.x += move.x;
	// viewProjection_.eye.y += move.y;
	// viewProjection_.eye.z += move.z;

	////行列の再計算
	// viewProjection_.UpdateMatrix();

	//注視点の移動処理
	//注視点の移動ベクトル
	// XMFLOAT3 tMove = {0, 0, 0};

	////注視点の移動速度
	// const float kTatgetSpeed = 0.2f;

	////押した方向で移動ベクトルを変更
	// if (input_->PushKey(DIK_LEFT)) {
	//	tMove = {-kTatgetSpeed, 0, 0};
	// } else if (input_->PushKey(DIK_RIGHT)) {
	//	tMove = {kTatgetSpeed, 0, 0};
	// }

	////注視点番号の切り替え
	// if (input_->TriggerKey(DIK_SPACE)) {
	//	if (tViewNum < 2) {
	//		tViewNum++;
	//	} else {
	//		tViewNum = 0;
	//	}
	// }

	////注視点移動（ベクトルの加算）
	// viewProjection_.target.x = worldTransform_[tViewNum].translation_.x;
	// viewProjection_.target.y = worldTransform_[tViewNum].translation_.y;
	// viewProjection_.target.z = worldTransform_[tViewNum].translation_.z;

	////行列の再計算
	// viewProjection_.UpdateMatrix();

	////上方向回転処理
	////上方向の回転速度[ラジアン/frame]
	// const float kUpRotSpeed = 0.05f;

	////押した方向で移動ベクトルを変更
	// if (input_->PushKey(DIK_SPACE)) {
	//	viewAngle += kUpRotSpeed;
	//	//2πを超えたら0に戻す
	//	viewAngle = fmodf(viewAngle, XM_2PI);
	// }

	////上方向ベクトルを計算(半径1の円周上の座標)
	// viewProjection_.up = {cosf(viewAngle), sinf(viewAngle), 0.0f};

	////行列の再計算
	// viewProjection_.UpdateMatrix();
	// viewProjection_.UpdateMatrix();

	////クリップ距離変更
	////上下キーでニアクリップ距離を増減
	// if (input_->PushKey(DIK_UP)) {
	//	viewProjection_.nearZ += 0.1f;
	// } else if (input_->PushKey(DIK_DOWN)) {
	//	viewProjection_.nearZ -= 0.1f;
	// }

	// viewProjection_.UpdateMatrix();

	//////////////////////
	// キャラクター移動処理 //
	//////////////////////
	
	////キャラクターの移動ベクトル
	// XMFLOAT3 move = {0, 0, 0};

	//回転速度
	const float rotSpeed = 0.05f;

	//押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_LEFT)) {
		worldTransform_[PartId::Root].rotation_.y += rotSpeed;
	} else if (input_->PushKey(DIK_RIGHT)) {
		worldTransform_[PartId::Root].rotation_.y -= rotSpeed;
	}

	//回転した角度をもとに正面ベクトル代入
	frontVector.x = sinf(worldTransform_[PartId::Root].rotation_.y);
	frontVector.z = cosf(worldTransform_[PartId::Root].rotation_.y);

	//キャラクターの移動速度
	const float kCharacterSpeed = 0.2f;

	//正面ベクトルの方向に移動速度をかけて移動させる
	if (input_->PushKey(DIK_UP)) {
		worldTransform_[PartId::Root].translation_.x += frontVector.x * kCharacterSpeed;
		worldTransform_[PartId::Root].translation_.z += frontVector.z * kCharacterSpeed;
	} else if (input_->PushKey(DIK_DOWN)) {
		worldTransform_[PartId::Root].translation_.x -= frontVector.x * kCharacterSpeed;
		worldTransform_[PartId::Root].translation_.z -= frontVector.z * kCharacterSpeed;
	}

	//行列の再計算
	worldTransform_[PartId::Root].UpdateMatrix();

	//位置リセット
	if (input_->PushKey(DIK_R)) {
		worldTransform_[PartId::Root].translation_.x = 0.0f;
		worldTransform_[PartId::Root].translation_.y = 0.0f;
		worldTransform_[PartId::Root].translation_.z = 0.0f;

		worldTransform_[PartId::Root].rotation_.x = 0.0f;
		worldTransform_[PartId::Root].rotation_.y = 0.0f;
		worldTransform_[PartId::Root].rotation_.z = 0.0f;
	}

	////注視点移動(ベクトルの加算)
	//worldTransform_[PartId::Root].translation_.x += move.x;
	//worldTransform_[PartId::Root].translation_.y += move.y;
	//worldTransform_[PartId::Root].translation_.z += move.z;

	//上半身回転処理
	//半身の回転速度[ラジアン/frame]
	const float kChestRotSpeed = 0.05f;

	//押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_U)) {
		worldTransform_[PartId::Chest].rotation_.y -= kChestRotSpeed;
	} else if (input_->PushKey(DIK_I)) {
		worldTransform_[PartId::Chest].rotation_.y += kChestRotSpeed;
	}

	//下半身回転処理
	//押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_J)) {
		worldTransform_[PartId::Hip].rotation_.y -= kChestRotSpeed;
	} else if (input_->PushKey(DIK_K)) {
		worldTransform_[PartId::Hip].rotation_.y += kChestRotSpeed;
	}

	//行列更新
	worldTransform_[PartId::Root].UpdateMatrix(); //必ず親から
	worldTransform_[PartId::Spine].UpdateMatrix();
	worldTransform_[PartId::Chest].UpdateMatrix();
	worldTransform_[PartId::Head].UpdateMatrix();
	worldTransform_[PartId::ArmL].UpdateMatrix();
	worldTransform_[PartId::ArmR].UpdateMatrix();
	worldTransform_[PartId::Hip].UpdateMatrix();
	worldTransform_[PartId::LegL].UpdateMatrix();
	worldTransform_[PartId::LegR].UpdateMatrix();
	worldTransform_[PartId::Oppai].UpdateMatrix();

	//デバッグ用表示
	/*debugText_->SetPos(50, 50);
	debugText_->Printf(
	  "eye:(%f,%f,%f)", viewProjection_.eye.x, viewProjection_.eye.y, viewProjection_.eye.z);

	debugText_->SetPos(50, 70);
	debugText_->Printf(
	  "target:(%f,%f,%f)", viewProjection_.target.x,
viewProjection_.target.y,viewProjection_.target.z);

	debugText_->SetPos(50, 90);
	debugText_->Printf(
	  "up:(%f,%f,%f)", viewProjection_.up.x, viewProjection_.up.y, viewProjection_.up.z);
<<<<<<< Updated upstream
=======

	debugText_->SetPos(50, 110);
	debugText_->Printf("fovAngleY(Degree):%f", XMConvertToDegrees(viewProjection_.fovAngleY));*/

	debugText_->SetPos(50, 150);
	debugText_->Printf("frontVector:(%f,%f,%f)", frontVector.x, frontVector.y, frontVector.z);

	debugText_->SetPos(50, 300);
	debugText_->Printf(
	  "Root.Rota:(%f,%f,%f)", worldTransform_[PartId::Root].rotation_.x,
	  worldTransform_[PartId::Root].rotation_.y, worldTransform_[PartId::Oppai].translation_.z);
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
	// 100個描画
	/*for (size_t i = 0; i < _countof(worldTransform_); i++) {
	    model_->Draw(worldTransform_[i], viewProjection_, textureHandle_);
	}*/

	//親子描画
	model_->Draw(worldTransform_[PartId::Root], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Spine], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Chest], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Head], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::ArmL], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::ArmR], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Hip], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::LegL], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::LegR], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Oppai], viewProjection_, textureHandle_);

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
