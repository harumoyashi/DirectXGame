#include "Audio.h"

#include "WinApp.h"
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
	//dwChannelMask = 0x00000033; //チャンネルのビットマスク
	dwChannelMask = SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_LEFT;
	masterVoice->GetChannelMask(&dwChannelMask);

	deviceDetails.InputChannels = 2;
	masterVoice->GetVoiceDetails(&deviceDetails);

	//エフェクト作成
	CreateFX(__uuidof(FXReverb), &pXAPO); // FXReverb:リバーブエフェクト

	// XAUDIO2_EFFECT_DESCRIPTOR構造体にデータを設定
	descriptor.InitialState = false; //エフェクト有効にするか
	descriptor.OutputChannels = 2;   //チャネルの数
	descriptor.pEffect = pXAPO;      // IUnknownインターフェイスへのポインタ

	// XAUDIO2_EFFECT_CHAIN構造体
	chain.EffectCount = 1;
	chain.pEffectDescriptors = &descriptor;

	//エフェクト情報などを適用するためのサブミックスボイス生成
	xAudio2_->CreateSubmixVoice(
	  &pSubmixVoice, 1, deviceDetails.InputSampleRate, 0, 0, nullptr, 0);	//最後&chainにするとエフェクトついたり

	//////////////////////
	//	X3DAudio初期化	//
	//////////////////////
	X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);

	memset(&DSPSettings, 0, sizeof(X3DAUDIO_DSP_SETTINGS));

	// X3DAUDIO_DSP_SETTING構造体のインスタンスを作成
	FLOAT32* matrix = new FLOAT32[deviceDetails.InputChannels]; //ここ一帯不安すぎ
	matrix[0] = {0.1f};
	matrix[1] = {0.5f};
	//ソースチャンネルの数
	DSPSettings.SrcChannelCount = Emitter.ChannelCount;
	DSPSettings.DstChannelCount = deviceDetails.InputChannels;
	//音量格納する行列
	DSPSettings.pMatrixCoefficients = matrix;

	memset(&Listener, 0, sizeof(X3DAUDIO_LISTENER));
	memset(&Emitter, 0, sizeof(X3DAUDIO_EMITTER));

	//向き、位置、ベクトルの代入
	Emitter.OrientFront = EOrientFront;
	Emitter.OrientTop = EOrientTop;
	Emitter.Position = EPosition;
	Emitter.Velocity = EVelocity;
	Listener.OrientFront = LOrientFront;
	Listener.OrientTop = LOrientTop;
	Listener.Position = LPosition;
	Listener.Velocity = LVelocity;

	//場所ごとのボリュームの設定
	volumePoints[0].Distance = 0.0f;
	volumePoints[0].DSPSetting = 0.0f;
	volumePoints[1].Distance = 0.2f;
	volumePoints[1].DSPSetting = 0.0f;
	volumePoints[2].Distance = 0.3f;
	volumePoints[2].DSPSetting = 0.0f;
	volumePoints[3].Distance = 0.4f;
	volumePoints[3].DSPSetting = 0.0f;
	volumePoints[4].Distance = 0.5f;
	volumePoints[4].DSPSetting = 0.0f;
	volumePoints[5].Distance = 0.6f;
	volumePoints[5].DSPSetting = 0.0f;
	volumePoints[6].Distance = 0.7f;
	volumePoints[6].DSPSetting = 0.0f;
	volumePoints[7].Distance = 0.8f;
	volumePoints[7].DSPSetting = 0.0f;
	volumePoints[8].Distance = 0.9f;
	volumePoints[8].DSPSetting = 0.0f;
	volumePoints[9].Distance = 1.0f;
	volumePoints[9].DSPSetting = 0.0f;
	volumeCurve.PointCount = 10;
	volumeCurve.pPoints = volumePoints;

	//場所ごとのリバーブの設定
	reverbPoints[0].Distance = 0.0f;
	reverbPoints[0].DSPSetting = 0.7f;
	reverbPoints[1].Distance = 0.2f;
	reverbPoints[1].DSPSetting = 0.78f;
	reverbPoints[2].Distance = 0.3f;
	reverbPoints[2].DSPSetting = 0.85f;
	reverbPoints[3].Distance = 0.4f;
	reverbPoints[3].DSPSetting = 1.0f;
	reverbPoints[4].Distance = 0.5f;
	reverbPoints[4].DSPSetting = 1.0f;
	reverbPoints[5].Distance = 0.6f;
	reverbPoints[5].DSPSetting = 0.6f;
	reverbPoints[6].Distance = 0.7f;
	reverbPoints[6].DSPSetting = 0.4f;
	reverbPoints[7].Distance = 0.8f;
	reverbPoints[7].DSPSetting = 0.25f;
	reverbPoints[8].Distance = 0.9f;
	reverbPoints[8].DSPSetting = 0.11f;
	reverbPoints[9].Distance = 1.0f;
	reverbPoints[9].DSPSetting = 0.0f;
	reverbCurve.PointCount = 10;
	reverbCurve.pPoints = reverbPoints;

	// emitterConeの初期化
	emitterCone.InnerAngle = X3DAUDIO_PI / 2;
	emitterCone.OuterAngle = X3DAUDIO_PI;
	emitterCone.InnerVolume = 1.0f;
	emitterCone.OuterVolume = 0.0f;
	emitterCone.InnerReverb = 1.0f;
	emitterCone.OuterReverb = 0.0f;

	//エミッタ構造体のインスタンスを作成
	Emitter.ChannelCount = 1;
	Emitter.CurveDistanceScaler = FLT_MAX;
	Emitter.InnerRadius = 2.0f;
	Emitter.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
	Emitter.ChannelRadius = 10.0f;
	Emitter.DopplerScaler = 1.0f;
	Emitter.pVolumeCurve = &volumeCurve;
	Emitter.pReverbCurve = &reverbCurve;
	Emitter.pCone = &emitterCone;

	/*Listener.Position =
	  X3DAUDIO_VECTOR(float(WinApp::kWindowWidth / 2), float(WinApp::kWindowHeight / 2), 0);
	Emitter.Position =
	  X3DAUDIO_VECTOR(float(WinApp::kWindowWidth / 2), float((WinApp::kWindowHeight / 2) - 100), 0);*/

	//サウンドデータに格納する情報の初期化
	format.fmt.nChannels = 2;          // 1だとモノラル,2以上だとステレオ
	format.fmt.nSamplesPerSec = 44100; //以下は周波数とかデータ転送速度とか難しい奴(いじらないほうがいいかも)
	format.fmt.nAvgBytesPerSec = 44100 * 2;
	format.fmt.nBlockAlign = 2;
	format.fmt.wBitsPerSample = 16;
	format.fmt.cbSize = 0;

	//リバーブの反射の仕方的な
	XAPOParameters.Diffusion = FXREVERB_MAX_DIFFUSION;
	//リバーブの反射する場所の距離的な
	XAPOParameters.RoomSize = FXREVERB_DEFAULT_ROOMSIZE;

	//サウンドデータ読み込み
	uint32_t soundDetaHandle_ = LoadWave("fanfare.wav");
	//音声再生
	PlayWave(soundDetaHandle_, true);
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

	// soundDataの情報を代入してく
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

	 XAUDIO2_SEND_DESCRIPTOR sendDescriptors[2] = {};
	sendDescriptors[0].Flags = XAUDIO2_SEND_USEFILTER;
	sendDescriptors[0].pOutputVoice = pSubmixVoice;
	sendDescriptors[1].Flags = XAUDIO2_SEND_USEFILTER;
	sendDescriptors[1].pOutputVoice = masterVoice;
	const XAUDIO2_VOICE_SENDS sendList = {2, sendDescriptors};

	// 波形フォーマットを元にSourceVoiceの生成
	result = xAudio2_->CreateSourceVoice(
	  &pSourceVoice, &soundData.wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback_, &sendList);
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
	    X3DAUDIO_CALCULATE_REVERB | X3DAUDIO_CALCULATE_EMITTER_ANGLE,
	  &DSPSettings); //紫の文字:どの種類の計算を有効にするか,DSPSettings:計算結果受け取るポインタ

	//ソースの音声にボリュームの値を適用
	pSourceVoice->SetOutputMatrix(
	  masterVoice, 1, deviceDetails.InputChannels, DSPSettings.pMatrixCoefficients);

	/*float outputMatrix[2] = {0.0f, 0.05f};
	pSourceVoice->SetOutputMatrix(masterVoice, 1, 2, outputMatrix);
	pSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 2, outputMatrix);*/

	/*float SourceVoiceChannelVolumes[1] = {1.0};
	result = pSourceVoice->SetChannelVolumes(0, SourceVoiceChannelVolumes);
	assert(SUCCEEDED(result));*/


	//FLOAT32* matrix = new FLOAT32[format.fmt.nChannels*2];
	//if (matrix == NULL)
	//	return false;
	//bool matrixAvailable = true;
	//switch (format.fmt.nChannels) {
	//case 4: // 4.0
	//	    // Speaker \ Left Source           Right Source
	//		/*Front L*/ matrix[0] = 1.0000f;	matrix[1] = 0.0000f;
	//		/*Front R*/ matrix[2] = 0.0000f;	matrix[3] = 1.0000f;
	//		/*Back  L*/ matrix[4] = 1.0000f;	matrix[5] = 0.0000f;
	//		/*Back  R*/ matrix[6] = 0.0000f;	matrix[7] = 1.0000f;
	//	break;
	//case 5: // 5.0
	//	    // Speaker \ Left Source           Right Source
	//		/*Front L*/ matrix[0] = 1.0000f;	matrix[1] = 0.0000f;
	//		/*Front R*/ matrix[2] = 0.0000f;	matrix[3] = 1.0000f;
	//		/*Front C*/ matrix[4] = 0.7071f;	matrix[5] = 0.7071f;
	//		/*Side  L*/ matrix[6] = 1.0000f;	matrix[7] = 0.0000f;
	//		/*Side  R*/ matrix[8] = 0.0000f;	matrix[9] = 1.0000f;
	//	break;
	//case 6: // 5.1
	//	    // Speaker \ Left Source           Right Source
	//		/*Front L*/ matrix[0] = 1.0000f;	matrix[1] = 0.0000f;
	//		/*Front R*/ matrix[2] = 0.0000f;	matrix[3] = 1.0000f;
	//		/*Front C*/ matrix[4] = 0.7071f;	matrix[5] = 0.7071f;
	//		/*LFE    */ matrix[6] = 0.0000f;	matrix[7] = 0.0000f;
	//		/*Side  L*/ matrix[8] = 1.0000f;	matrix[9] = 0.0000f;
	//		/*Side  R*/ matrix[10] = 0.0000f;	matrix[11] = 1.0000f;
	//	break;
	//case 7: // 6.1
	//	    // Speaker \ Left Source           Right Source
	//		/*Front L*/ matrix[0] = 1.0000f;	matrix[1] = 0.0000f;
	//		/*Front R*/ matrix[2] = 0.0000f;	matrix[3] = 1.0000f;
	//		/*Front C*/ matrix[4] = 0.7071f;	matrix[5] = 0.7071f;
	//		/*LFE    */ matrix[6] = 0.0000f;	matrix[7] = 0.0000f;
	//		/*Side  L*/ matrix[8] = 1.0000f;	matrix[9] = 0.0000f;
	//		/*Side  R*/ matrix[10] = 0.0000f;	matrix[11] = 1.0000f;
	//		/*Back  C*/ matrix[12] = 0.7071f;	matrix[13] = 0.7071f;
	//	break;
	//case 8: // 7.1
	//	    // Speaker \ Left Source           Right Source
	//		/*Front L*/ matrix[0] = 1.0000f;	matrix[1] = 0.0000f;
	//		/*Front R*/ matrix[2] = 0.0000f;	matrix[3] = 1.0000f;
	//		/*Front C*/ matrix[4] = 0.7071f;	matrix[5] = 0.7071f;
	//		/*LFE    */ matrix[6] = 0.0000f;	matrix[7] = 0.0000f;
	//		/*Back  L*/ matrix[8] = 1.0000f;	matrix[9] = 0.0000f;
	//		/*Back  R*/ matrix[10] = 0.0000f;	matrix[11] = 1.0000f;
	//		/*Side  L*/ matrix[12] = 1.0000f;	matrix[13] = 0.0000f;
	//		/*Side  R*/ matrix[14] = 0.0000f;	matrix[15] = 1.0000f;
	//	break;
	//default:
	//	matrixAvailable = false;
	//	break;
	//}
	//if (matrixAvailable) {
	//	result = pSourceVoice->SetOutputMatrix(masterVoice, 1, format.fmt.nChannels, matrix);
	//	assert(SUCCEEDED(result));
	//}
	//free(matrix);
	//matrix = NULL;

	//ピッチも適用
	/*pSourceVoice->SetFrequencyRatio(DSPSettings.DopplerFactor);*/

	//計算されたリバーブレベルをサブミックスの音声に適用
	/*pSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);*/

	//エフェクトチェーンをvoiceに適用
	pSourceVoice->SetEffectChain(&chain);
	pXAPO->Release();

	result = pSourceVoice->SetEffectParameters(0, &XAPOParameters, sizeof(FXREVERB_PARAMETERS));
	assert(SUCCEEDED(result));

	//計算された低パスフィルターの直接係数をソースの音声に適用
	XAUDIO2_FILTER_PARAMETERS FilterParameters = {
	  LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * DSPSettings.LPFDirectCoefficient), 1.0f};
	pSourceVoice->SetFilterParameters(&FilterParameters);

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	pSourceVoice->SetVolume(volume);

	result = pSourceVoice->Start(0);

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
