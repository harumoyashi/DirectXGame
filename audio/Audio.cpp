#include "Audio.h"

#include <cassert>
#include <fstream>
#include <windows.h>

#pragma comment(lib, "xaudio2.lib")

void Audio::XAudio2VoiceCallback::OnBufferEnd(THIS_ void* pBufferContext) {

	Voice* voice = reinterpret_cast<Voice*>(pBufferContext);
	// 再生リストから除外
	Audio::GetInstance()->voices_.erase(voice);
}

Audio* Audio::GetInstance() {
	static Audio instance;

	return &instance;
}

void Audio::Initialize(const std::string& directoryPath) {
	directoryPath_ = directoryPath;

	HRESULT result;

	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));

	indexSoundData_ = 0u;
	indexVoice_ = 0u;

	//チャネルマスク(どっから音出すかのビット情報)設定
	DWORD dwChannelMask;
	masterVoice->GetChannelMask(&dwChannelMask);

	//エフェクト作成
	CreateFX(__uuidof(FXReverb), &pXAPO);	//FXReverb:リバーブエフェクト

	//XAUDIO2_EFFECT_DESCRIPTOR構造体にデータを設定
	descriptor.InitialState = true;	//エフェクト有効にするか
	descriptor.OutputChannels = 2;	//チャネルの数
	descriptor.pEffect = pXAPO;		//IUnknownインターフェイスへのポインタ

	//XAUDIO2_EFFECT_CHAIN構造体
	chain.EffectCount = 1;
	chain.pEffectDescriptors = &descriptor;

	//リバーブの反射の仕方的な
	XAPOParameters.Diffusion = FXREVERB_MAX_DIFFUSION;
	//リバーブの反射する場所の距離的な
	XAPOParameters.RoomSize = FXREVERB_DEFAULT_ROOMSIZE;

	//X3DAudio初期化
	X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);

	Listener.pCone = nullptr;
	Emitter.InnerRadius = 2.0f;
	Emitter.ChannelRadius = 10.0f;
	Emitter.CurveDistanceScaler = 1.0f;

	//向き、位置、ベクトルの代入
	Emitter.OrientFront = EOrientFront;
	Emitter.OrientTop = EOrientTop;
	Emitter.Position = EPosition;
	Emitter.Velocity = EVelocity;
	Listener.OrientFront = LOrientFront;
	Listener.OrientTop = LOrientTop;
	Listener.Position = LPosition;
	Listener.Velocity = LVelocity;

	//エミッタ構造体のインスタンスを作成
	Emitter.ChannelCount = 1;
	Emitter.CurveDistanceScaler = FLT_MIN;

	//X3DAUDIO_DSP_SETTING構造体のインスタンスを作成
	FLOAT32* matrix = new FLOAT32[descriptor.OutputChannels]; //ここ一帯不安すぎ
	DSPSettings.SrcChannelCount = 1;
	DSPSettings.DstChannelCount = descriptor.OutputChannels;
	DSPSettings.pMatrixCoefficients = matrix;
}

void Audio::Finalize() {
	// XAudio2解放
	xAudio2_.Reset();
	// 音声データ解放
	for (auto& soundData : soundDatas_) {
		Unload(&soundData);
	}
}

uint32_t Audio::LoadWave(const std::string& fileName) {
	assert(indexSoundData_ < kMaxSoundData);
	uint32_t handle = indexSoundData_;
	// 読み込み済みサウンドデータを検索
	auto it = std::find_if(soundDatas_.begin(), soundDatas_.end(), [&](const auto& soundData) {
		return soundData.name_ == fileName;
	});
	if (it != soundDatas_.end()) {
		// 読み込み済みサウンドデータの要素番号を取得
		handle = static_cast<uint32_t>(std::distance(soundDatas_.begin(), it));
		return handle;
	}

	// ディレクトリパスとファイル名を連結してフルパスを得る
	bool currentRelative = false;
	if (2 < fileName.size()) {
		currentRelative = (fileName[0] == '.') && (fileName[1] == '/');
	}
	std::string fullpath = currentRelative ? fileName : directoryPath_ + fileName;

	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(fullpath, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクか Broadcast Wave Formatを検出した場合。
	while (_strnicmp(data.id, "junk", 4) == 0 || _strnicmp(data.id, "bext", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}
	if (_strnicmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();

	// 書き込むサウンドデータの参照
	SoundData& soundData = soundDatas_.at(handle);

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	soundData.name_ = fileName;

	indexSoundData_++;

	return handle;
}

void Audio::Unload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

uint32_t Audio::PlayWave(uint32_t soundDataHandle, bool loopFlag, float volume) {
	HRESULT result;

	assert(soundDataHandle <= soundDatas_.size());

	// サウンドデータの参照を取得
	SoundData& soundData = soundDatas_.at(soundDataHandle);
	// 未読み込みの検出
	assert(soundData.bufferSize != 0);

	uint32_t handle = indexVoice_;

	// 波形フォーマットを元にSourceVoiceの生成
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex, 0, 2.0f, &voiceCallback_);
	assert(SUCCEEDED(result));

	// 再生中データ
	Voice* voice = new Voice();
	voice->handle = handle;
	voice->sourceVoice = pSourceVoice;
	// 再生中データコンテナに登録
	voices_.insert(voice);

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.pContext = voice;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (loopFlag) {
		// 無限ループ
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	//音声の新しい設定を計算
	X3DAudioCalculate(
	  X3DInstance, &Listener, &Emitter,
	  X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT |
	    X3DAUDIO_CALCULATE_REVERB,
	  &DSPSettings); //紫の文字:どの種類の計算を有効にするか,DSPSettings:計算結果受け取るポインタ

	//ソースの音声にボリュームの値を適用
	pSourceVoice->SetOutputMatrix(
	  masterVoice, 1, Emitter.ChannelCount, DSPSettings.pMatrixCoefficients);
	//ピッチも適用
	pSourceVoice->SetFrequencyRatio(DSPSettings.DopplerFactor);

	//計算されたリバーブレベルをサブミックスの音声に適用
	pSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);

	//エフェクトチェーンをvoiceに適用
	pSourceVoice->SetEffectChain(&chain);
	pXAPO->Release();

	result = pSourceVoice->SetEffectParameters(0, &XAPOParameters, sizeof(FXREVERB_PARAMETERS));

	//計算された低パスフィルターの直接係数をソースの音声に適用
	XAUDIO2_FILTER_PARAMETERS FilterParameters = {
	  LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * DSPSettings.LPFDirectCoefficient), 1.0f};
	pSourceVoice->SetFilterParameters(&FilterParameters);

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	pSourceVoice->SetVolume(volume);
	result = pSourceVoice->Start();

	indexVoice_++;

	return handle;
}

void Audio::StopWave(uint32_t voiceHandle) {

	// 再生中リストから検索
	auto it = std::find_if(
	  voices_.begin(), voices_.end(), [&](Voice* voice) { return voice->handle == voiceHandle; });
	// 発見
	if (it != voices_.end()) {
		(*it)->sourceVoice->DestroyVoice();

		voices_.erase(it);
	}
}

bool Audio::IsPlaying(uint32_t voiceHandle) {
	// 再生中リストから検索
	auto it = std::find_if(
	  voices_.begin(), voices_.end(), [&](Voice* voice) { return voice->handle == voiceHandle; });
	// 発見。再生終わってるのかどうかを判断
	if (it != voices_.end()) {
		XAUDIO2_VOICE_STATE state{};
		(*it)->sourceVoice->GetState(&state);
		return state.SamplesPlayed != 0;
	}
	return false;
}

void Audio::SetVolume(uint32_t voiceHandle, float volume) {
	// 再生中リストから検索
	auto it = std::find_if(
	  voices_.begin(), voices_.end(), [&](Voice* voice) { return voice->handle == voiceHandle; });
	// 発見
	if (it != voices_.end()) {
		(*it)->sourceVoice->SetVolume(volume);
	}
}
