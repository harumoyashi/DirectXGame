#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>

#include <Audio.h>

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
		// x,y,z方向のスケーリングを設定
		worldTransform_[Emitter].scale_ = {8.0f, 8.0f, 8.0f};
		worldTransform_[Listener].scale_ = {5.0f, 5.0f, 5.0f};

		// x,y,z軸周りの回転角を設定
		worldTransform_[i].rotation_ = {0.0f, 0.0f, 0.0f};

		// x,y,z軸周りの平行移動を設定
		worldTransform_[Emitter].translation_ = audio_->Emitter.Position;
		worldTransform_[Listener].translation_ = audio_->Listener.Position;

		//ワールドトランスフォームの初期化
		worldTransform_[i].Initialize();
	}

	//カメラ視点座標設定
	viewProjection_.eye = {0, 50.0f, -150.0f};

	//ビュープロジェクションの初期化
	viewProjection_.Initialize();
}

void GameScene::Update() {
	//オブジェクトの移動ベクトル
	XMFLOAT3 move = {0, 0, 0};

	const float moveSpeed = 0.5f;

	////平行移動(x,y,z)
	// if (input_->PushKey(DIK_LEFT)) {
	//	worldTransform_[Emitter].translation_.x -= moveSpeed;
	// } else if (input_->PushKey(DIK_RIGHT)) {
	//	worldTransform_[Emitter].translation_.x += moveSpeed;
	// }

	// if (input_->PushKey(DIK_S)) {
	//	worldTransform_[Emitter].translation_.y -= moveSpeed;
	// } else if (input_->PushKey(DIK_W)) {
	//	worldTransform_[Emitter].translation_.y += moveSpeed;
	// }

	// if (input_->PushKey(DIK_DOWN)) {
	//	worldTransform_[Emitter].translation_.z -= moveSpeed;
	// } else if (input_->PushKey(DIK_UP)) {
	//	worldTransform_[Emitter].translation_.z += moveSpeed;
	// }

	//回転移動(横)
	const float rotSpeed = 0.02f;

	//回転ベクトル変更
	if (input_->PushKey(DIK_U)) {
		viewAngle += rotSpeed;
		viewAngle = fmodf(viewAngle, XM_2PI);
	} else if (input_->PushKey(DIK_I)) {
		viewAngle -= rotSpeed;
		viewAngle = fmodf(viewAngle, XM_2PI);
	}

	//回転ベクトルの加算
	worldTransform_[Listener].translation_.x = cosf(viewAngle) * 40.0f;
	worldTransform_[Listener].translation_.z = sinf(viewAngle) * 40.0f;

	//行列の再計算
	worldTransform_[Listener].UpdateMatrix();

	audio_->Emitter.Position = worldTransform_[Emitter].translation_;
	audio_->Listener.Position = worldTransform_[Listener].translation_;

	////1Way音量調整//
	////Y座標の距離計算
	// float distance;
	// distance = (abs(audio_->Emitter.Position.x - audio_->Listener.Position.x) +
	//            abs(audio_->Emitter.Position.y - audio_->Listener.Position.y))/2;

	////距離をもとに音量パラメーター設定
	// float volume;
	// if (distance <= 0.0f) {
	//	volume = 1.0f;
	// } else if (distance > 100.0f) {
	//	volume = 0.0f;
	// } else {
	//	volume = 1.0f - distance / 100.0f;
	// }

	////音量調整
	// audio_->SetVolume(soundDetaHandle_, volume);

	//エフェクトのON,OFF
	if (input_->TriggerKey(DIK_SPACE)) {
		if (audio_->descriptor.InitialState == false) {
			audio_->pSourceVoice->EnableEffect(0);
			audio_->descriptor.InitialState = true;
		} else {
			audio_->pSourceVoice->DisableEffect(0);
			audio_->descriptor.InitialState = false;
		}
	}

	/*if (input_->TriggerKey(DIK_UP)) {
		if (reverbIndex < 22) {
			reverbIndex++;
		}
		audio_->SetReverb(reverbIndex);
	} else if (input_->TriggerKey(DIK_DOWN)) {
		if (reverbIndex > 1) {
			reverbIndex--;
		}
		audio_->SetReverb(reverbIndex);
	}*/

	if (input_->TriggerKey(DIK_R)) {
		audio_->XAPOParameters.Diffusion = FXREVERB_MIN_DIFFUSION;
		audio_->pSourceVoice->SetEffectParameters(
		  0, &audio_->XAPOParameters, sizeof(FXREVERB_PARAMETERS));
	}

	/*float volume3 = 0.2f;
	float volume2 = 1.0f;

	audio_->pSourceVoice->SetChannelVolumes(SPEAKER_FRONT_LEFT, &volume3);
	audio_->pSourceVoice->SetChannelVolumes(SPEAKER_FRONT_RIGHT, &volume2);*/

	debugText_->SetPos(20, 20);
	debugText_->Printf(
	  "EmitterPos:\nx:%f\ny:%f\nz:%f", audio_->Emitter.Position.x, audio_->Emitter.Position.y,
	  audio_->Emitter.Position.z);

	debugText_->SetPos(20, 50);
	debugText_->Printf(
	  "WorldEmitterPos:\nx:%f\ny:%f\nz:%f", worldTransform_[Emitter].translation_.x,
	  worldTransform_[Emitter].translation_.y, worldTransform_[Emitter].translation_.z);
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
