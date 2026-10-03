#include <jni.h>
#include <cstring>
#include <sstream>
#include <thread>
#include <vector>
#include <dlfcn.h>

#include "game.hpp"
#include "local.hpp"
#include "stdinclude.hpp"

#include <dobby.h>

#include "hook.h"

#include "log.h"

#include "zygoteloader/dex.hpp"
#include "zygoteloader/zygoteloader.h"

#include "logger/logger.hpp"
#include "config.hpp"

#include "il2cpp_dump.h"

#include "il2cpp/il2cpp_symbols.hpp"

#include "msgpack/msgpack_modify.hpp"

#include "notification/MediaNotificationManager.hpp"

#include "localify/NotificationManager.hpp"
#include "localify/UIParts.hpp"
#include "localify/LiveUtils.hpp"

#include "scripts/ScriptInternal.hpp"

#include "scripts/mscorlib/System/Collections/Generic/Dictionary.hpp"
#include "scripts/mscorlib/System/ValueTuple.hpp"

#include "scripts/Cute.Cri.Assembly/Cute/Cri/MoviePlayerHandle.hpp"
#include "scripts/Cute.Cri.Assembly/Cute/Cri/MoviePlayerForUI.hpp"

#include "scripts/UnityEngine.CoreModule/UnityEngine/RectTransform.hpp"
#include "scripts/UnityEngine.CoreModule/UnityEngine/Screen.hpp"
#include "scripts/UnityEngine.CoreModule/UnityEngine/SceneManagement/Scene.hpp"
#include "scripts/UnityEngine.CoreModule/UnityEngine/Vector2Int.hpp"

#include "scripts/umamusume/Gallop/GameSystem.hpp"
#include "scripts/umamusume/Gallop/GraphicSettings.hpp"
#include "scripts/umamusume/Gallop/UIManager.hpp"
#include "scripts/umamusume/Gallop/SceneManager.hpp"
#include "scripts/umamusume/Gallop/Screen.hpp"
#include "scripts/umamusume/Gallop/StoryViewController.hpp"
#include "scripts/umamusume/Gallop/RaceCameraManager.hpp"
#include "scripts/umamusume/Gallop/Localize.hpp"
#include "scripts/umamusume/Gallop/LowResolutionCameraUtil.hpp"
#include "scripts/umamusume/Gallop/LiveViewController.hpp"
#include "scripts/umamusume/Gallop/Live/Director.hpp"

#include "scripts/Plugins/AnimateToUnity/AnRootManager.hpp"

using namespace std;
using namespace logger;

namespace {
    JNIEnv *env;
    Resource *classesDex;

    void patch_game_assembly();

    void init_il2cpp() {
        if (config::dump_il2cpp) {
            il2cpp_dump();
        }

        il2cpp_symbols::init_defaults();
        il2cpp_symbols::call_init_callbacks();

        il2cpp_symbols::late_init_callbacks.emplace_back(patch_game_assembly);
    }

    void *il2cpp_init_addr = nullptr;
    void *il2cpp_init_orig = nullptr;

    static bool il2cpp_init_hook(const char *domain_name) {
        DobbyDestroy(il2cpp_init_addr);
        const auto result = reinterpret_cast<decltype(il2cpp_init_hook) *>(il2cpp_init_addr)(
                domain_name);

        auto unityVersion = il2cpp_resolve_icall_type<Il2CppString *(*)()>(
                "UnityEngine.Application::get_unityVersion")();

        if (IL2CPP_BASIC_STRING(unityVersion->chars).contains(IL2CPP_STRING("2020"))) {
            Game::CurrentUnityVersion = Game::UnityVersion::Unity20;
        } else {
            Game::CurrentUnityVersion = Game::UnityVersion::Unity22;
        }

        il2cpp_symbols::il2cpp_domain = il2cpp_domain_get();
        init_il2cpp();
        return result;
    }

    void StartTickFrame();

    void SetBGCanvasScalerSize() {
        auto bgManager = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)()>("umamusume.dll",
                                                                                 "Gallop",
                                                                                 "BGManager",
                                                                                 "get_Instance",
                                                                                 0)();
        if (bgManager) {
            auto _mainBgField = il2cpp_class_get_field_from_name(bgManager->klass, "_mainBg");
            Il2CppObject *_mainBg;
            il2cpp_field_get_value(bgManager, _mainBgField, &_mainBg);

            if (_mainBg) {
                auto _currentBgWidthField = il2cpp_class_get_field_from_name(bgManager->klass,
                                                                             "_currentBgWidth");
                int _currentBgWidth;
                il2cpp_field_get_value(bgManager, _currentBgWidthField, &_currentBgWidth);

                auto _currentBgHeightField = il2cpp_class_get_field_from_name(bgManager->klass,
                                                                              "_currentBgHeight");
                int _currentBgHeight;
                il2cpp_field_get_value(bgManager, _currentBgHeightField, &_currentBgHeight);

                if (!_currentBgWidth || !_currentBgHeight) {
                    return;
                }

                float ratio =
                        static_cast<float>(_currentBgWidth) / static_cast<float>(_currentBgHeight);

                int width = Gallop::Screen::Width();
                int height = Gallop::Screen::Height();

                if (_currentBgWidth < _currentBgHeight) {
                    _currentBgHeight = height;
                    _currentBgWidth = static_cast<int>(_currentBgHeight * ratio);
                } else {
                    _currentBgWidth = width;
                    _currentBgHeight = static_cast<int>(_currentBgWidth / ratio);
                }

                il2cpp_field_set_value(bgManager, _currentBgWidthField, &_currentBgWidth);
                il2cpp_field_set_value(bgManager, _currentBgHeightField, &_currentBgHeight);

                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(bgManager->klass,
                                                                             "RecalcBgSize", 0)(
                        bgManager);
            }
        }
    }

    void ResizeMiniDirector() {
        Il2CppArraySize_t<Il2CppObject *> *miniDirectors;
        miniDirectors = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "MiniDirector"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (miniDirectors) {
            for (int i = 0; i < miniDirectors->max_length; i++) {
                auto obj = miniDirectors->vector[i];

                if (obj) {
                    auto state = il2cpp_symbols::get_method_pointer<int (*)(Il2CppObject *)>(
                            obj->klass, "get_State", 0)(obj);

                    if (state > 0) {
                        auto DirectorUI = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(obj->klass, "get_DirectorUI", 0)(obj);
                        auto cameraController = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(obj->klass, "get_CameraController", 0)(obj);

                        if (DirectorUI && cameraController) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                    DirectorUI->klass, "ResetTextureSize", 0)(DirectorUI);

                            auto TextureResolution = il2cpp_symbols::get_method_pointer<UnityEngine::Vector2Int(*)(
                                    Il2CppObject *)>(DirectorUI->klass, "get_TextureResolution", 0)(
                                    DirectorUI);

                            auto _cameraField = il2cpp_class_get_field_from_name(
                                    cameraController->klass, "_camera");
                            Il2CppObject *_camera;
                            il2cpp_field_get_value(cameraController, _cameraField, &_camera);

                            if (_camera) {
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            UnityEngine::Vector2Int)>(
                                        cameraController->klass, "ResizeRenderTexture", 1)(
                                        cameraController, TextureResolution);

                                auto _renderTextureField = il2cpp_class_get_field_from_name(
                                        cameraController->klass, "_renderTexture");
                                Il2CppObject *_renderTexture;
                                il2cpp_field_get_value(cameraController, _renderTextureField,
                                                       &_renderTexture);

                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            Il2CppObject *)>(
                                        DirectorUI->klass, "SetRenderTexture", 1)(DirectorUI,
                                                                                  _renderTexture);
                            }
                        }
                    }
                }
            }
        }
    }


    Il2CppObject *delayTweener;

    void RemakeTextures() {
        auto uiManager = Gallop::UIManager::Instance();

        auto graphicSettings = Gallop::GraphicSettings::Instance();
        if (!graphicSettings) {
            return;
        }

        graphicSettings.Update3DRenderTexture();

        Il2CppArraySize_t<Il2CppObject *> *renders;
        renders = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "CutInImageEffectPostRender"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (renders) {
            for (int i = 0; i < renders->max_length; i++) {
                auto obj = renders->vector[i];

                if (obj) {
                    auto buffer = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                            Il2CppObject *)>(obj->klass, "get_FrameBuffer", 0)(obj);
                    if (buffer) {
                        if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                                buffer->klass, "get_ColorBuffer", 0)(buffer)) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                    buffer->klass, "RemakeRenderTexture", 0)(buffer);
                        }
                    }
                }
            }
        }

        Il2CppArraySize_t<Il2CppObject *> *cuts;
        cuts = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "LimitBreakCut"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (cuts) {
            for (int i = 0; i < cuts->max_length; i++) {
                auto obj = cuts->vector[i];

                if (obj) {
                    auto _frameBufferField = il2cpp_class_get_field_from_name(obj->klass,
                                                                              "_frameBuffer");
                    Il2CppObject *_frameBuffer;
                    il2cpp_field_get_value(obj, _frameBufferField, &_frameBuffer);

                    if (_frameBuffer) {
                        if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                                _frameBuffer->klass, "get_ColorBuffer", 0)(_frameBuffer)) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                    _frameBuffer->klass, "RemakeRenderTexture", 0)(_frameBuffer);
                        }
                    }
                }
            }
        }

        Il2CppArraySize_t<Il2CppObject *> *raceEffect;
        raceEffect = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "RaceImageEffect"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (raceEffect) {
            for (int i = 0; i < raceEffect->max_length; i++) {
                auto obj = raceEffect->vector[i];

                if (obj) {
                    auto get_FrameBuffer = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                            Il2CppObject *)>(obj->klass, "get_FrameBuffer", 0);
                    if (get_FrameBuffer) {
                        auto buffer = get_FrameBuffer(obj);
                        if (buffer) {
                            auto _drawPassField = il2cpp_class_get_field_from_name(buffer->klass,
                                                                                   "_drawPass");
                            int *_drawPass;
                            il2cpp_field_get_value(buffer, _drawPassField, &_drawPass);

                            if (!_drawPass) {
                                int defPass = 0;
                                il2cpp_field_set_value(buffer, _drawPassField, &defPass);
                            }


                            if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                    Il2CppObject *)>(buffer->klass, "get_ColorBuffer", 0)(buffer)) {
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                        buffer->klass, "RemakeRenderTexture", 0)(buffer);
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        /*Il2CppArraySize_t<Il2CppObject*>* storyEffect;
        storyEffect = UnityEngine::Object::FindObjectsByType(
            GetRuntimeType("umamusume.dll", "Gallop", "StoryImageEffect"), UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (storyEffect)
        {
            for (int i = 0; i < storyEffect->max_length; i++)
            {
                auto obj = storyEffect->vector[i];

                if (obj)
                {
                    auto buffer = il2cpp_symbols::get_method_pointer<Il2CppObject * (*)(Il2CppObject*)>(obj->klass, "get_FrameBuffer", 0)(obj);
                    if (buffer)
                    {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject*)>(buffer->klass, "RemakeRenderTexture", 0)(buffer);
                    }
                }
            }
        }*/

        Il2CppArraySize_t<Il2CppObject *> *lowResCameras;
        lowResCameras = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "LowResolutionCameraBase"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (lowResCameras) {
            for (int i = 0; i < lowResCameras->max_length; i++) {
                auto obj = lowResCameras->vector[i];

                if (obj) {
                    auto method = il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                            obj->klass, "ResizeRenderTextureSize", 0);
                    if (method) {
                        method(obj);
                    }
                }
            }
        }

        Il2CppArraySize_t<Il2CppObject *> *liveTheaterCharaSelects;
        liveTheaterCharaSelects = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "LiveTheaterCharaSelect"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (liveTheaterCharaSelects) {
            for (int i = 0; i < liveTheaterCharaSelects->max_length; i++) {
                auto obj = liveTheaterCharaSelects->vector[i];

                if (obj) {
                    auto _sceneField = il2cpp_class_get_field_from_name(obj->klass, "_scene");
                    Il2CppObject *_scene;
                    il2cpp_field_get_value(obj, _sceneField, &_scene);

                    if (_scene) {
                        auto camera = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(_scene->klass, "GetCamera", 0)(_scene);
                        auto texture = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(camera->klass, "get_RenderTexture", 0)(camera);

                        auto _formationAllField = il2cpp_class_get_field_from_name(obj->klass,
                                                                                   "_formationAll");
                        Il2CppObject *_formationAll;
                        il2cpp_field_get_value(obj, _formationAllField, &_formationAll);

                        if (_formationAll) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                        Il2CppObject *)>(
                                    _formationAll->klass, "SetRenderTex", 1)(_formationAll,
                                                                             texture);
                        }

                        auto _formationMainField = il2cpp_class_get_field_from_name(obj->klass,
                                                                                    "_formationMain");
                        Il2CppObject *_formationMain;
                        il2cpp_field_get_value(obj, _formationMainField, &_formationMain);

                        if (_formationMain) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                        Il2CppObject *)>(
                                    _formationMain->klass, "SetRenderTex", 1)(_formationMain,
                                                                              texture);
                        }

                        // TODO: reposition
                    }
                }
            }
        }

        Il2CppArraySize_t<Il2CppObject *> *miniDirectors;
        miniDirectors = UnityEngine::Object::FindObjectsByType(
                GetRuntimeType("umamusume.dll", "Gallop", "MiniDirector"),
                UnityEngine::FindObjectsInactive::Include, UnityEngine::FindObjectsSortMode::None);

        if (miniDirectors && miniDirectors->max_length) {

            if (delayTweener) {
                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, bool)>("DOTween.dll",
                                                                                   "DG.Tweening",
                                                                                   "TweenExtensions",
                                                                                   "Complete", 2)(
                        delayTweener, true);
            }

            auto callback = CreateDelegateWithClass(
                    il2cpp_symbols::get_class("DOTween.dll", "DG.Tweening", "TweenCallback"),
                    uiManager, *([](Il2CppObject *self) {
                        ResizeMiniDirector();
                        delayTweener = nullptr;
                    }));

            // Delay 50ms
            delayTweener = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(float,
                                                                                Il2CppDelegate *,
                                                                                bool)>(
                    "DOTween.dll", "DG.Tweening", "DOVirtual", "DelayedCall", 3)(0.05,
                                                                                 &callback->delegate,
                                                                                 true);
        }

        auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();

        if (controller) {
            if (controller->klass->name == "SingleModeMainViewController"s) {
                auto ScenarioController = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                        Il2CppObject *)>(controller->klass, "get_ScenarioController", 0)(
                        controller);

                if (ScenarioController && ScenarioController->klass->name ==
                                          "SingleModeMainViewScenarioBreedersController"s) {
                    auto IsStoryActive = il2cpp_symbols::get_method_pointer<bool (*)(
                            Il2CppObject *)>(controller->klass, "get_IsStoryActive", 0)(controller);

                    if (!IsStoryActive) {
                        auto trainingController = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(controller->klass, "get_TrainingController", 0)(
                                controller);
                        if (!il2cpp_symbols::get_method_pointer<bool (*)(Il2CppObject *)>(
                                trainingController->klass, "get_IsInTraining", 0)(
                                trainingController)) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                    ScenarioController->klass, "PlayCutIn", 0)(ScenarioController);
                        }
                    }
                }
            }

            if (controller->klass->name == "PhotoStudioViewController"s) {
                auto _photoStudioTopCharaViewerField = il2cpp_class_get_field_from_name(
                        controller->klass, "_photoStudioTopCharaViewer");
                Il2CppObject *_photoStudioTopCharaViewer;
                il2cpp_field_get_value(controller, _photoStudioTopCharaViewerField,
                                       &_photoStudioTopCharaViewer);

                if (_photoStudioTopCharaViewer) {
                    auto _lowResolutionCameraField = il2cpp_class_get_field_from_name(
                            _photoStudioTopCharaViewer->klass, "_lowResolutionCamera");
                    Il2CppObject *_lowResolutionCamera;
                    il2cpp_field_get_value(_photoStudioTopCharaViewer, _lowResolutionCameraField,
                                           &_lowResolutionCamera);

                    if (_lowResolutionCamera) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                    Il2CppObject *)>(
                                _photoStudioTopCharaViewer->klass, "OnCreateTexture", 1)(
                                _photoStudioTopCharaViewer, _lowResolutionCamera);
                    }
                }
            }

            if (controller->klass->name == "FanRaidViewController"s) {
                auto _fanRaidTopSequenceField = il2cpp_class_get_field_from_name(controller->klass,
                                                                                 "_fanRaidTopSequence");
                Il2CppObject *_fanRaidTopSequence;
                il2cpp_field_get_value(controller, _fanRaidTopSequenceField, &_fanRaidTopSequence);

                if (_fanRaidTopSequence) {
                    auto _frameBufferField = il2cpp_class_get_field_from_name(
                            _fanRaidTopSequence->klass, "_frameBuffer");
                    Il2CppObject *_frameBuffer;
                    il2cpp_field_get_value(_fanRaidTopSequence, _frameBufferField, &_frameBuffer);

                    if (_frameBuffer) {
                        if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                                _frameBuffer->klass, "get_ColorBuffer", 0)(_frameBuffer)) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                    _frameBuffer->klass, "RemakeRenderTexture", 0)(_frameBuffer);
                        }
                    }
                }
            }

            if (controller->klass->name == "GachaMainViewController"s) {
                auto _contextField = il2cpp_class_get_field_from_name(controller->klass,
                                                                      "_context");
                Il2CppObject *_context;
                il2cpp_field_get_value(controller, _contextField, &_context);

                if (_context) {
                    auto FrameBufferField = il2cpp_class_get_field_from_name(_context->klass,
                                                                             "FrameBuffer");
                    Il2CppObject *FrameBuffer;
                    il2cpp_field_get_value(_context, FrameBufferField, &FrameBuffer);

                    if (FrameBuffer) {
                        if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                                FrameBuffer->klass, "get_ColorBuffer", 0)(FrameBuffer)) {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                    FrameBuffer->klass, "RemakeRenderTexture", 0)(FrameBuffer);
                        }
                    }
                }
            }

            if (controller->klass->name == "SingleModeSuccessionCutViewController"s ||
                controller->klass->name == "EpisodeMainUnlockRaceCutinViewController"s ||
                controller->klass->name == "SingleModeSuccessionEventViewController"s) {
                auto _resultField = il2cpp_class_get_field_from_name(controller->klass, "_result");
                Il2CppObject *_result;
                il2cpp_field_get_value(controller, _resultField, &_result);

                if (_result) {
                    auto _resultCameraField = il2cpp_class_get_field_from_name(_result->klass,
                                                                               "_resultCamera");
                    Il2CppObject *_resultCamera;
                    il2cpp_field_get_value(_result, _resultCameraField, &_resultCamera);

                    if (_resultCamera) {
                        auto texture = uiManager.UITexture();
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                    Il2CppObject *)>(
                                _resultCamera->klass, "set_targetTexture", 1)(_resultCamera,
                                                                              texture);
                    }
                }
            }

            if (string(controller->klass->name).ends_with("PaddockViewController")) {
                auto _frameBufferField = il2cpp_class_get_field_from_name(controller->klass,
                                                                          "_frameBuffer");
                Il2CppObject *_frameBuffer;
                il2cpp_field_get_value(controller, _frameBufferField, &_frameBuffer);

                if (_frameBuffer) {
                    if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                            _frameBuffer->klass, "get_ColorBuffer", 0)(_frameBuffer)) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                _frameBuffer->klass, "RemakeRenderTexture", 0)(_frameBuffer);
                    }
                }
            }

            if (config::freeform_window) {
                if (string(controller->klass->name).ends_with("LiveViewController")) {
                    auto view = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                            Il2CppObject *)>(controller->klass, "GetViewBase", 0)(controller);
                    auto _fullPortraitRootField = il2cpp_class_get_field_from_name(view->klass,
                                                                                   "_fullPortraitRoot");

                    if (_fullPortraitRootField) {
                        Il2CppObject *_fullPortraitRoot;
                        il2cpp_field_get_value(view, _fullPortraitRootField, &_fullPortraitRoot);

                        if (_fullPortraitRoot) {
                            UnityEngine::GameObject(_fullPortraitRoot).SetActive(false);
                        }
                    }
                }
            }
        }

        auto storyManager = GetSingletonInstance(
                il2cpp_symbols::get_class("umamusume.dll", "Gallop", "StoryManager"));
        if (storyManager) {
            auto storySceneController = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)()>(
                    "umamusume.dll", "Gallop", "StoryManager", "get_StorySceneController",
                    IgnoreNumberOfArguments)();
            if (storySceneController) {
                auto DisplayMode = il2cpp_symbols::get_method_pointer<int (*)(Il2CppObject *)>(
                        storySceneController->klass, "get_DisplayMode", 0)(storySceneController);

                Gallop::StoryViewController storyViewController = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                        Il2CppObject *)>("umamusume.dll", "Gallop", "StoryManager",
                                         "get_ViewController", 0)(storyManager);

                auto IsSingleModeOrGallery = storyViewController.IsSingleModeOrGallery();

                auto scene = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                        storySceneController->klass, "GetSceneBase", 0)(storySceneController);

                if (!IsSingleModeOrGallery) {
                    storyViewController.SetDisplayMode(DisplayMode);
                } else {
                    if (DisplayMode == 1) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                storySceneController->klass, "SetDisplayAreaPortrait", 0)(
                                storySceneController);
                    } else {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                storySceneController->klass, "SetDisplayAreaFullScreen", 0)(
                                storySceneController);
                    }

                    int drawDirection = 6;

                    if (DisplayMode == 0 || DisplayMode == 3) {
                        drawDirection = 0;
                    } else if (DisplayMode == 1) {
                        drawDirection = 7;
                    }

                    auto _lowResolutionCameraListField = il2cpp_class_get_field_from_name(
                            storySceneController->klass, "_lowResolutionCameraList");
                    Il2CppObject *_lowResolutionCameraList;
                    il2cpp_field_get_value(storySceneController, _lowResolutionCameraListField,
                                           &_lowResolutionCameraList);

                    if (_lowResolutionCameraList) {
                        FieldInfo *_itemsField = il2cpp_class_get_field_from_name(
                                _lowResolutionCameraList->klass, "_items");
                        Il2CppArraySize_t<Il2CppObject *> *_items;
                        il2cpp_field_get_value(_lowResolutionCameraList, _itemsField, &_items);

                        for (int i = 0; i < _items->max_length; i++) {
                            auto lowResolutionCamera = _items->vector[i];

                            if (lowResolutionCamera) {
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            int)>(
                                        lowResolutionCamera->klass, "ChangeDirection", 1)(
                                        lowResolutionCamera, drawDirection);
                            }
                        }
                    }

                    auto FrameBufferDisplayMode = il2cpp_symbols::get_method_pointer<int (*)(
                            int)>("umamusume.dll", "Gallop", "LowResolutionCameraUtil",
                                  "GetDrawPass", 1)(DisplayMode);

                    auto FrameBuffer = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                            Il2CppObject *)>(storySceneController->klass, "get_FrameBuffer", 0)(
                            storySceneController);

                    if (il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(Il2CppObject *)>(
                            FrameBuffer->klass, "get_ColorBuffer", 0)(FrameBuffer)) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, int)>(
                                FrameBuffer->klass, "RemakeRenderTexture", 1)(FrameBuffer,
                                                                              FrameBufferDisplayMode);
                    }

                    // il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject*, int)>(storySceneController->klass, "UpdateFovFactor", 1)(storySceneController, DisplayMode);

                    auto FullScreenImageRendererField = il2cpp_class_get_field_from_name(
                            scene->klass, "FullScreenImageRenderer");
                    Il2CppObject *FullScreenImageRenderer;
                    il2cpp_field_get_value(scene, FullScreenImageRendererField,
                                           &FullScreenImageRenderer);

                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                            FullScreenImageRenderer->klass, "ForceRender", 0)(
                            FullScreenImageRenderer);
                }

                storyViewController.SetupUIOnChangeOrientation();
            }
        }
    }

    void ResizeMoviePlayer() {
        auto movieManager = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)()>(
                "Cute.Cri.Assembly.dll", "Cute.Cri", "MovieManager", "get_Instance",
                IgnoreNumberOfArguments)();

        if (movieManager) {
            auto playerDicField = il2cpp_class_get_field_from_name(movieManager->klass,
                                                                   "playerDic");
            Il2CppObject *playerDic;
            il2cpp_field_get_value(movieManager, playerDicField, &playerDic);

            if (playerDic) {
                auto entriesField = il2cpp_class_get_field_from_name(playerDic->klass, "_entries");
                if (!entriesField) {
                    entriesField = il2cpp_class_get_field_from_name(playerDic->klass, "entries");
                }

                Il2CppArraySize_t<System::Collections::Generic::Dictionary<Cute::Cri::MoviePlayerHandle, Il2CppObject *>::Entry> *entries;
                il2cpp_field_get_value(playerDic, entriesField, &entries);

                if (entries) {
                    for (int i = 0; i < entries->max_length; i++) {
                        auto entry = entries->vector[i];

                        auto player = entry.value;

                        if (player) {
                            auto gameObject = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                    Il2CppObject *)>(player->klass, "get_gameObject", 0)(player);

                            if (gameObject) {
                                auto transform = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                        Il2CppObject *)>(gameObject->klass, "get_transform", 0)(
                                        gameObject);

                                if (transform) {
                                    auto parent = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                            Il2CppObject *)>(transform->klass, "get_parent", 0)(
                                            transform);

                                    if (parent) {
                                        auto parentGameObject = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                                Il2CppObject *)>(parent->klass, "get_gameObject",
                                                                 0)(parent);
                                        auto getComponents = il2cpp_symbols::get_method_pointer<Il2CppArraySize *(*)(
                                                Il2CppObject *, Il2CppType *, bool, bool, bool,
                                                bool, Il2CppObject *)>(parentGameObject->klass,
                                                                       "GetComponentsInternal", 6);

                                        if (UnityEngine::Object::Name(parent)->chars ==
                                            il2cppstring(IL2CPP_STRING("MainCanvas"))) {
                                            if (auto klass = il2cpp_symbols::get_class(
                                                    "umamusume.dll", "Gallop", "StoryMovieView")) {
                                                auto array1 = getComponents(parentGameObject,
                                                                            reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                                                                    klass)), true,
                                                                            true, false, false,
                                                                            nullptr);

                                                if (array1) {
                                                    if (array1->max_length > 0) {
                                                        auto fullPlayer = il2cpp_object_new(
                                                                il2cpp_symbols::get_class(
                                                                        "umamusume.dll", "Gallop",
                                                                        "StoryFullMoviePlayer"));
                                                        auto _handleField = il2cpp_class_get_field_from_name(
                                                                fullPlayer->klass, "_handle");
                                                        il2cpp_field_set_value(fullPlayer,
                                                                               _handleField,
                                                                               &entry.key);

                                                        il2cpp_symbols::get_method_pointer<void (*)(
                                                                Il2CppObject *, int)>(
                                                                fullPlayer->klass,
                                                                "AdjustMovieSize", 1)(fullPlayer,
                                                                                      Gallop::Screen::IsVertical()
                                                                                      ? 0 : 1);

                                                        return;
                                                    }
                                                }
                                            }

                                            auto array2 = getComponents(parentGameObject,
                                                                        reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                                                                "umamusume.dll",
                                                                                "Gallop",
                                                                                "StoryView")), true,
                                                                        true, false, false,
                                                                        nullptr);

                                            if (array2) {
                                                if (array2->max_length > 0) {
                                                    auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();

                                                    auto _wipeControllerField = il2cpp_class_get_field_from_name(
                                                            controller->klass, "_wipeController");
                                                    Il2CppObject *_wipeController;
                                                    il2cpp_field_get_value(controller,
                                                                           _wipeControllerField,
                                                                           &_wipeController);

                                                    if (_wipeController) {
                                                        auto _moviePlayerField = il2cpp_class_get_field_from_name(
                                                                _wipeController->klass,
                                                                "_moviePlayer");
                                                        Il2CppObject *_moviePlayer;
                                                        il2cpp_field_get_value(_wipeController,
                                                                               _moviePlayerField,
                                                                               &_moviePlayer);

                                                        if (_moviePlayer) {
                                                            auto StoryTimelineController = il2cpp_symbols::get_class(
                                                                    "umamusume.dll", "Gallop",
                                                                    "StoryTimelineController");
                                                            auto CurrentDisplayModeField = il2cpp_class_get_field_from_name(
                                                                    StoryTimelineController->klass,
                                                                    "CurrentDisplayMode");
                                                            int CurrentDisplayMode;
                                                            il2cpp_field_static_get_value(
                                                                    CurrentDisplayModeField,
                                                                    &CurrentDisplayMode);

                                                            if (CurrentDisplayMode == 3 &&
                                                                !Gallop::Screen::IsVertical()) {
                                                                int tmpMode = 2;
                                                                il2cpp_field_static_get_value(
                                                                        CurrentDisplayModeField,
                                                                        &tmpMode);
                                                            }

                                                            il2cpp_symbols::get_method_pointer<void (*)(
                                                                    Il2CppObject *)>(
                                                                    _moviePlayer->klass,
                                                                    "AdjustScreenSize", 0)(
                                                                    _moviePlayer);

                                                            il2cpp_field_static_set_value(
                                                                    CurrentDisplayModeField,
                                                                    &CurrentDisplayMode);
                                                        }
                                                    }
                                                    return;
                                                }
                                            }

                                            auto newSize = il2cpp_symbols::get_method_pointer<UnityEngine::Vector2(*)()>(
                                                    "umamusume.dll", "Gallop",
                                                    "MovieScreenSizeHelper",
                                                    "GetMovieTargetCanvasSize",
                                                    IgnoreNumberOfArguments)();

                                            auto criPlayer = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                                    Il2CppObject *)>(player->klass, "get_Player",
                                                                     0)(player);
                                            if (criPlayer) {
                                                auto status = il2cpp_symbols::get_method_pointer<int (*)(
                                                        Il2CppObject *)>(criPlayer->klass,
                                                                         "get_status", 0)(
                                                        criPlayer);
                                                if (status == 5) {
                                                    Cute::Cri::MoviePlayerForUI(
                                                            player).AdjustScreenSize(newSize, true);
                                                }
                                            }

                                        } else if (parent->klass->name == "RectTransform"s) {
                                            auto parentGameObject = UnityEngine::RectTransform(
                                                    parent).gameObject();
                                            auto array = parentGameObject.GetComponentsInChildren(
                                                    GetRuntimeType("umamusume.dll", "Gallop",
                                                                   "PartsEpisodeList"), false);

                                            if (array) {
                                                for (int j = 0; j < array->max_length; j++) {
                                                    auto obj = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                                            Il2CppObject *, long index)>(
                                                            "mscorlib.dll", "System", "Array",
                                                            "GetValue", 1)(array, j);
                                                    if (!obj) continue;

                                                    auto newSize = il2cpp_symbols::get_method_pointer<UnityEngine::Vector2(*)(
                                                            Il2CppObject *)>(obj->klass,
                                                                             "CalcMovieRectSize",
                                                                             0)(obj);

                                                    il2cpp_symbols::get_method_pointer<void (*)(
                                                            Il2CppObject *, UnityEngine::Vector2)>(
                                                            parent->klass, "set_sizeDelta", 1)(
                                                            parent, newSize);

                                                    auto criPlayer = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                                            Il2CppObject *)>(player->klass,
                                                                             "get_Player", 0)(
                                                            player);

                                                    if (criPlayer) {
                                                        auto status = il2cpp_symbols::get_method_pointer<int (*)(
                                                                Il2CppObject *)>(criPlayer->klass,
                                                                                 "get_status", 0)(
                                                                criPlayer);
                                                        if (status == 5) {
                                                            Cute::Cri::MoviePlayerForUI(
                                                                    player).AdjustScreenSize(
                                                                    newSize, true);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void WaitForEndOfFrame(void (*fn)()) {
        try {
            auto gameSystem = Gallop::GameSystem::Instance();
            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, Il2CppDelegate *)>(
                    "umamusume.dll", "Gallop", "MonoBehaviourExtension", "WaitForEndFrame", 2)(
                    gameSystem, CreateDelegateStatic(fn));
        } catch (const Il2CppExceptionWrapper &ex) {
            LOGW("WaitForEndOfFrame error: %s", il2cpp_u8(ex.ex->message->chars).data());
            PrintStackTrace();
        }
    }

    void WaitForEndOfFrame(Il2CppObject *target, void (*fn)(Il2CppObject *self)) {
        try {
            auto delegate = &CreateUnityAction(target, fn)->delegate;
            auto gameSystem = Gallop::GameSystem::Instance();
            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, Il2CppDelegate *)>(
                    "umamusume.dll", "Gallop", "MonoBehaviourExtension", "WaitForEndFrame", 2)(
                    gameSystem, delegate);
        } catch (const Il2CppExceptionWrapper &ex) {
            LOGW("WaitForEndOfFrame error: %s", il2cpp_u8(ex.ex->message->chars).data());
            PrintStackTrace();
        }
    }

    void ResizeWindow(int _updateWidth, int _updateHeight) {
        if (_updateWidth < 72 || _updateHeight < 72) {
            return;
        }

        static int updateWidth;
        static int updateHeight;

        updateWidth = _updateWidth;
        updateHeight = _updateHeight;

        auto refreshRate = UnityEngine::RefreshRate{0, 0};
        UnityEngine::Screen::SetResolution_Injected(updateWidth, updateHeight,
                                                    UnityEngine::FullScreenMode::FullScreenWindow,
                                                    &refreshRate);

        WaitForEndOfFrame(*[] {
            WaitForEndOfFrame(*[] {
                const auto contentWidth = UnityEngine::Screen::width();
                const auto contentHeight = UnityEngine::Screen::height();

                auto ratio = static_cast<float>(contentWidth) / static_cast<float>(contentHeight);

                auto lastWidth = updateWidth;
                auto lastHeight = updateHeight;

                const auto _aspectRatio = contentWidth / contentHeight;

                const auto isPortrait = contentWidth < contentHeight;

                const auto unityWidth = UnityEngine::Screen::width();
                const auto unityHeight = UnityEngine::Screen::height();

                const auto isUnityPortrait = unityWidth < unityHeight;

                Gallop::Screen::OriginalScreenWidth(isUnityPortrait ? contentHeight : contentWidth);
                Gallop::Screen::OriginalScreenHeight(
                        isUnityPortrait ? contentWidth : contentHeight);

                auto tapEffectController = GetSingletonInstance(
                        il2cpp_symbols::get_class("umamusume.dll", "Gallop",
                                                  "TapEffectController"));

                auto uiManager = Gallop::UIManager::Instance();

                if (uiManager) {
                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                            tapEffectController->klass, "Disable", 0)(tapEffectController);

                    uiManager.SetCameraSizeByOrientation(UnityEngine::ScreenOrientation::Portrait);
                }

                auto anRootManager = AnimateToUnity::AnRootManager::Instance();

                if (anRootManager) {
                    anRootManager.ScreenRate(_aspectRatio);
                }

                if (uiManager) {
                    auto gameObject = uiManager.gameObject();

                    auto transform = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                            Il2CppObject *)>(gameObject, "get_transform", 0)(gameObject);

                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                UnityEngine::Vector3)>(
                            transform->klass, "set_localScale", 1)(transform,
                                                                   UnityEngine::Vector3{1, 1, 1});

                    if (tapEffectController) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                tapEffectController->klass, "Enable", 0)(tapEffectController);
                    }

                    Il2CppArraySize_t<Il2CppObject *> *canvasScalerList;
                    canvasScalerList = UnityEngine::Object::FindObjectsByType(
                            GetRuntimeType("UnityEngine.UI.dll", "UnityEngine.UI", "CanvasScaler"),
                            UnityEngine::FindObjectsInactive::Include,
                            UnityEngine::FindObjectsSortMode::None);

                    for (int i = 0; i < canvasScalerList->max_length; i++) {
                        auto canvasScaler = canvasScalerList->vector[i];
                        if (canvasScaler) {
                            auto gameObject = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                    Il2CppObject *)>(canvasScaler->klass, "get_gameObject", 0)(
                                    canvasScaler);

                            const auto keepActive = il2cpp_symbols::get_method_pointer<bool (*)(
                                    Il2CppObject *)>(gameObject->klass, "get_activeSelf", 0)(
                                    gameObject);

                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, bool)>(
                                    gameObject->klass, "SetActive", 1)(gameObject, true);

                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, bool)>(
                                    gameObject->klass, "SetActive", 1)(gameObject, keepActive);

                            auto scaleMode = il2cpp_symbols::get_method_pointer<int (*)(
                                    Il2CppObject *)>(canvasScaler->klass, "get_uiScaleMode", 0)(
                                    canvasScaler);

                            if (scaleMode == 1) {
                                if (isPortrait) {
                                    const auto scale = min(config::freeform_ui_scale_portrait,
                                                           max(1.0f,
                                                               static_cast<float>(contentHeight) *
                                                               config::runtime::ratioVertical) *
                                                           config::freeform_ui_scale_portrait);
                                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                                UnityEngine::Vector2)>(
                                            canvasScaler->klass, "set_referenceResolution", 1)(
                                            canvasScaler, UnityEngine::Vector2{
                                                    static_cast<float>(contentWidth / scale),
                                                    static_cast<float>(contentHeight / scale)});
                                } else {
                                    const auto scale = min(config::freeform_ui_scale_landscape,
                                                           max(1.0f,
                                                               static_cast<float>(contentWidth) /
                                                               config::runtime::ratioHorizontal) *
                                                           config::freeform_ui_scale_landscape);
                                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                                UnityEngine::Vector2)>(
                                            canvasScaler->klass, "set_referenceResolution", 1)(
                                            canvasScaler, UnityEngine::Vector2{
                                                    static_cast<float>(contentWidth / scale),
                                                    static_cast<float>(contentHeight / scale)});
                                }
                            }

                            if (scaleMode == 0) {
                                if (isPortrait) {
                                    const auto scale = min(config::freeform_ui_scale_portrait,
                                                           max(1.0f,
                                                               static_cast<float>(contentHeight) *
                                                               config::runtime::ratioVertical) *
                                                           config::freeform_ui_scale_portrait);
                                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                                float)>(
                                            canvasScaler->klass, "set_scaleFactor", 1)(canvasScaler,
                                                                                       scale);
                                } else {
                                    const auto scale = min(config::freeform_ui_scale_landscape,
                                                           max(1.0f,
                                                               static_cast<float>(contentWidth) /
                                                               config::runtime::ratioHorizontal) *
                                                           config::freeform_ui_scale_landscape);
                                    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                                float)>(
                                            canvasScaler->klass, "set_scaleFactor", 1)(canvasScaler,
                                                                                       scale);
                                }
                            }
                        }
                    }

                    SetBGCanvasScalerSize();
                }

                static int _contentWidth;
                static int _contentHeight;
                _contentWidth = contentWidth;
                _contentHeight = contentHeight;

                WaitForEndOfFrame(*[] {
                    const auto tapEffectController = GetSingletonInstance(
                            il2cpp_symbols::get_class("umamusume.dll", "Gallop",
                                                      "TapEffectController"));

                    const auto isPortrait = _contentWidth < _contentHeight;

                    auto uiManager = Gallop::UIManager::Instance();

                    if (uiManager) {
                        uiManager.SetupSafeArea();
                        uiManager.AdjustSafeArea();
                        auto _bgManager = uiManager._bgManager();
                        if (_bgManager) {
                            _bgManager.OnChangeResolutionByGraphicsSettings();
                        }

                        uiManager.CreateRenderTextureFromScreen();
                    }

                    RemakeTextures();

                    auto raceCameraManager = Gallop::RaceCameraManager::Instance();
                    if (raceCameraManager) {
                        raceCameraManager.SetupOrientation(isPortrait
                                                           ? Gallop::LowResolutionCameraUtil::DrawDirection::Portrait
                                                           : Gallop::LowResolutionCameraUtil::DrawDirection::Landscape);
                    }

                    const auto director = Gallop::Live::Director::Instance();
                    if (director) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, int)>(director,
                                                                                          "SetupOrientation",
                                                                                          1)(
                                director, isPortrait ? 2 : 1);

                        const auto ChampionsTextControllerField = il2cpp_class_get_field_from_name(
                                director, "ChampionsTextController");
                        Il2CppObject *ChampionsTextController;
                        il2cpp_field_get_value(director, ChampionsTextControllerField,
                                               &ChampionsTextController);

                        if (ChampionsTextController) {
                            const auto _flashPlayerField = il2cpp_class_get_field_from_name(
                                    ChampionsTextController->klass, "_flashPlayer");
                            Il2CppObject *_flashPlayer;
                            il2cpp_field_get_value(ChampionsTextController, _flashPlayerField,
                                                   &_flashPlayer);

                            const auto root = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                    Il2CppObject *)>(_flashPlayer->klass, "get_Root", 0)(
                                    _flashPlayer);

                            float scale = 1.0f;

                            if (_contentWidth < _contentHeight) {
                                scale = min(config::freeform_ui_scale_portrait, max(1.0f,
                                                                                    static_cast<float>(_contentHeight) *
                                                                                    config::runtime::ratioVertical) *
                                                                                config::freeform_ui_scale_portrait);
                            } else {
                                scale = min(config::freeform_ui_scale_landscape, max(1.0f,
                                                                                     static_cast<float>(_contentWidth) /
                                                                                     config::runtime::ratioHorizontal) *
                                                                                 config::freeform_ui_scale_landscape);
                            }

                            const auto availableWidth = static_cast<float>(_contentWidth) / scale;
                            const auto availableHeight = static_cast<float>(_contentHeight) / scale;

#ifdef _MSC_VER
                            auto width = ratio_16_9 * availableHeight;
#else
                            const auto width = availableWidth;
#endif
                            const auto height = availableHeight;

#ifdef _MSC_VER
                            if (width > availableWidth)
                            {
                                width = availableWidth;
                                height = width / ratio_16_9;
                            }
#endif

                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                        UnityEngine::Vector2)>(
                                    root->klass, "SetScreenReferenceSize", 1)(root,
                                                                              UnityEngine::Vector2{
                                                                                      width,
                                                                                      height});
                        }


                        const auto liveFlashController = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(director, "get_LiveFlashController", 0)(director);

                        if (liveFlashController) {
                            const auto _flashPlayerField = il2cpp_class_get_field_from_name(
                                    liveFlashController->klass, "_flashPlayer");

                            if (_flashPlayerField) {
                                Il2CppObject *_flashPlayer;
                                il2cpp_field_get_value(liveFlashController, _flashPlayerField,
                                                       &_flashPlayer);

                                const auto root = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                        Il2CppObject *)>(_flashPlayer->klass, "get_Root", 0)(
                                        _flashPlayer);

                                float scale = 1.0f;

                                if (_contentWidth < _contentHeight) {
                                    scale = min(config::freeform_ui_scale_portrait, max(1.0f,
                                                                                        static_cast<float>(_contentHeight) *
                                                                                        config::runtime::ratioVertical) *
                                                                                    config::freeform_ui_scale_portrait);
                                } else {
                                    scale = min(config::freeform_ui_scale_landscape, max(1.0f,
                                                                                         static_cast<float>(_contentWidth) /
                                                                                         config::runtime::ratioHorizontal) *
                                                                                     config::freeform_ui_scale_landscape);
                                }

                                const auto availableWidth =
                                        static_cast<float>(_contentWidth) / scale;
                                const auto availableHeight =
                                        static_cast<float>(_contentHeight) / scale;

#ifdef _MSC_VER
                                auto width = ratio_16_9 * availableHeight;
#else
                                const auto width = availableWidth;
#endif
                                const auto height = availableHeight;

#ifdef _MSC_VER
                                if (width > availableWidth)
                                {
                                    width = availableWidth;
                                    height = width / ratio_16_9;
                                }
#endif

                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            UnityEngine::Vector2)>(
                                        root->klass, "SetScreenReferenceSize", 1)(root,
                                                                                  UnityEngine::Vector2{
                                                                                          width,
                                                                                          height});
                            }
                        }
                    }

                    if (tapEffectController) {
                        il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *)>(
                                tapEffectController->klass, "RefreshAll", 0)(tapEffectController);
                    }

                    if (uiManager) {
                        uiManager.AdjustMissionClearContentsRootRect();
                        uiManager.AdjustSafeAreaToAnnounceRect();

                        /*Il2CppObject* _bgCamera = uiManager._bgCamera();

                        if (_bgCamera)
                        {
                            il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject*, Color_t)>(_bgCamera->klass, "set_backgroundColor", 1)(_bgCamera,
                                il2cpp_symbols::get_method_pointer<Color_t(*)()>("UnityEngine.CoreModule.dll", "UnityEngine", "Color", "get_clear", IgnoreNumberOfArguments)());
                        }*/
                    }

                    Gallop::Screen::OriginalScreenWidth(
                            isPortrait ? _contentHeight : _contentWidth);
                    Gallop::Screen::OriginalScreenHeight(
                            isPortrait ? _contentWidth : _contentHeight);
                });
            });
        });
    }

    void TickFrame() {
        try {
            if (config::unlock_size || config::freeform_window) {
                SetBGCanvasScalerSize();
            }

            if (config::freeform_window) {
                ResizeMoviePlayer();
            }

            auto sceneManager = Gallop::SceneManager::Instance();

            if (!sceneManager) {
                StartTickFrame();
                return;
            }

            const il2cppstring sceneName = sceneManager.GetCurrentSceneIdName()->chars;

            if (sceneName == IL2CPP_STRING("Live")) {
                auto controller = sceneManager.GetCurrentViewController();

                if (controller && controller->klass->name == "LiveViewController"s) {
                    const auto director = Gallop::Live::Director::Instance();
                    if (director) {
                        auto LiveCurrentTime = il2cpp_symbols::get_method_pointer<float (*)(
                                Il2CppObject *)>(director, "get_LiveCurrentTime", 0)(director);
                        const auto LiveTotalTime = il2cpp_symbols::get_method_pointer<float (*)(
                                Il2CppObject *)>(director, "get_LiveTotalTime", 0)(director);

                        updateMediaProgress(GetJNIEnv(), GetActivity(),
                                            static_cast<int64_t>(LiveCurrentTime * 1000),
                                            static_cast<int64_t>(LiveTotalTime * 1000));

                        auto sliderCommon = Localify::UIParts::GetOptionSlider("live_slider");

                        auto textCommon = Localify::UIParts::GetTextCommon("live_slider");

                        if (textCommon) {
                            const auto timeMin = static_cast<int>(LiveCurrentTime / 60);
                            const auto timeSec = static_cast<int>(fmodf(LiveCurrentTime, 60));

                            const auto timeMinIl2Cpp = to_string(timeMin);
                            const auto timeSecIl2Cpp = to_string(timeSec);

                            stringstream str;
                            str << setw(2) << setfill('0') << timeSecIl2Cpp;

                            textCommon.text(
                                    il2cpp_string_new((timeMinIl2Cpp + ":" + str.str()).data()));
                        }

                        auto textCommonTotal = Localify::UIParts::GetTextCommon(
                                "live_slider_total");

                        if (textCommonTotal) {
                            const auto timeMin = static_cast<int>(LiveTotalTime / 60);
                            const auto timeSec = static_cast<int>(fmodf(LiveTotalTime, 60));

                            const auto timeMinIl2Cpp = to_string(timeMin);
                            const auto timeSecIl2Cpp = to_string(timeSec);

                            stringstream str;
                            str << setw(2) << setfill('0') << timeSecIl2Cpp;

                            textCommonTotal.text(
                                    il2cpp_string_new((timeMinIl2Cpp + ":" + str.str()).data()));
                        }

                        if (config::live_playback_loop) {
                            if (LiveCurrentTime >= LiveTotalTime - 0.1f) {
                                LiveCurrentTime = 0;
                                Localify::LiveUtils::MoveLivePlayback(LiveCurrentTime);
                            }
                        }

                        try {
                            if (sliderCommon) {
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, float)>(
                                        sliderCommon->klass, "set_maxValue", 1)(sliderCommon,
                                                                                LiveTotalTime);
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, float)>(
                                        sliderCommon->klass, "SetValueWithoutNotify", 1)(
                                        sliderCommon, LiveCurrentTime);
                            }
                        } catch (const Il2CppExceptionWrapper &ex) {
                            cout << ex.ex->klass->name << ": ";
                            wcout << ex.ex->message << endl;
                        }
                    }
                }
            }

            if (sceneName == IL2CPP_STRING("Home")) {
                bool hasSetList = false;

                auto hubViewController = GetCurrentHubViewChildController();

                if (hubViewController && hubViewController->klass->name == "HomeViewController"s) {
                    auto topUi = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                            Il2CppObject *, int)>(hubViewController->klass, "GetTopUI",
                                                  1)->methodPointer(hubViewController, 10);
                    if (topUi) {
                        auto get_TempSetListPlayingData = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                                Il2CppObject *)>(topUi->klass, "get_TempSetListPlayingData", 0);
                        if (get_TempSetListPlayingData) {
                            auto data = get_TempSetListPlayingData->methodPointer(topUi);

                            auto IsPlayingField = il2cpp_class_get_field_from_name(data->klass,
                                                                                   "IsPlaying");
                            bool IsPlaying;
                            il2cpp_field_get_value(data, IsPlayingField, &IsPlaying);

                            hasSetList = IsPlaying;

                            if (IsPlaying) {
                                updateMediaPlayWhenReady(GetJNIEnv(), GetActivity(), true);

                                auto SetListIndexField = il2cpp_class_get_field_from_name(
                                        data->klass, "SetListIndex");
                                int SetListIndex;
                                il2cpp_field_get_value(data, SetListIndexField, &SetListIndex);

                                int MusicListCount = il2cpp_class_get_method_from_name_type<int (*)(
                                        Il2CppObject *)>(data->klass, "GetMusicListCount",
                                                         0)->methodPointer(data);

                                updateMediaNavigationButtons(GetJNIEnv(), GetActivity(),
                                                             SetListIndex < MusicListCount - 1,
                                                             SetListIndex > 0);

                                auto musicData = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                                        Il2CppObject *)>(data->klass, "GetMasterSetListMusicData",
                                                         0)->methodPointer(data);

                                auto MusicIdField = il2cpp_class_get_field_from_name(
                                        musicData->klass, "MusicId");
                                int MusicId;
                                il2cpp_field_get_value(musicData, MusicIdField, &MusicId);

                                MediaNotificationManager::UpdateMetadata(MusicId);
                            }
                        }
                    }
                }

                if (!hasSetList) {
                    auto workDataManager = GetSingletonInstance(
                            il2cpp_symbols::get_class("umamusume.dll", "Gallop",
                                                      "WorkDataManager"));

                    auto workJukeboxData = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                            Il2CppObject *)>(workDataManager->klass, "get_JukeboxData",
                                             0)->methodPointer(workDataManager);
                    auto currentBgmMusicId = il2cpp_class_get_method_from_name_type<int (*)(
                            Il2CppObject *)>(workJukeboxData->klass, "GetCurrentBgmMusicId",
                                             0)->methodPointer(workJukeboxData);

                    MediaNotificationManager::UpdateMetadata(currentBgmMusicId);

                    if (currentBgmMusicId) {
                        updateMediaPlayWhenReady(GetJNIEnv(), GetActivity(), true);
                    } else {
                        updateMediaPlayWhenReady(GetJNIEnv(), GetActivity(), false);
                    }
                }
            }

            if (sceneName == IL2CPP_STRING("Live")) {
                auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();

                if (controller && controller->klass->name == "LiveViewController"s) {
                    auto _stateField = il2cpp_class_get_field_from_name(controller->klass,
                                                                        "_state");
                    Gallop::LiveViewController::LiveState state;
                    il2cpp_field_get_value(controller, _stateField, &state);

                    bool IsStarted = false;
                    if (auto director = Gallop::Live::Director::Instance())
                    {
                        IsStarted = director.IsStarted();
                    }

                    if (IsStarted && state == Gallop::LiveViewController::LiveState::Play) {
                        updateMediaPlayWhenReady(GetJNIEnv(), GetActivity(), true);
                    } else {
                        updateMediaPlayWhenReady(GetJNIEnv(), GetActivity(), false);
                    }
                }
            }

            StartTickFrame();
        } catch (const Il2CppExceptionWrapper &ex) {
            LOGW("TickFrame error: %s", il2cpp_u8(ex.ex->message->chars).data());
        }
    }

    void StartTickFrame() {
        static auto tickFrameDelegate = CreateDelegateStatic(TickFrame);

        try {
            const auto GameSystem = Gallop::GameSystem::Instance();
            if (GameSystem) {
                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, Il2CppDelegate *)>(
                        "umamusume.dll", "Gallop", "MonoBehaviourExtension", "WaitForEndFrame", 2)(
                        GameSystem, tickFrameDelegate);
            }
        } catch (const Il2CppExceptionWrapper &ex) {
            LOGW("StartTickFrame error: %s", il2cpp_u8(ex.ex->message->chars).data());
        }
    }

    void patch_game_assembly() {
        LOGI("patch_game_assembly");

        if (config::dump_entries) {
            Gallop::Localize::DumpAllEntries();
        }

        const auto env = GetJNIEnv();
        const auto activity = GetActivity();

        if (config::freeform_window) {
            register_callback(env, activity);

            const auto windowMetricsCalculatorClass = env->GetObjectClass(
                    windowMetricsCalculator);

            const auto computeId = env->GetMethodID(windowMetricsCalculatorClass,
                                              "computeCurrentWindowMetrics",
                                              "(Landroid/app/Activity;)Landroidx/window/layout/WindowMetrics;");
            env->DeleteLocalRef(windowMetricsCalculatorClass);

            const auto metrics = env->CallObjectMethod(windowMetricsCalculator, computeId,
                                                 activity);
            const auto metricsClass = env->GetObjectClass(metrics);

            const auto getRectId = env->GetMethodID(metricsClass, "getBounds",
                                              "()Landroid/graphics/Rect;");
            const auto rect = env->CallObjectMethod(metrics, getRectId);

            env->DeleteLocalRef(metrics);
            env->DeleteLocalRef(metricsClass);

            const auto rectClass = env->GetObjectClass(rect);

            const auto widthId = env->GetMethodID(rectClass, "width", "()I");
            jint width = env->CallIntMethod(rect, widthId);

            const auto heightId = env->GetMethodID(rectClass, "height", "()I");
            jint height = env->CallIntMethod(rect, heightId);

            env->DeleteLocalRef(rect);
            env->DeleteLocalRef(rectClass);

            auto insets = getDisplayCutoutInsets(env, activity);
            const auto insetsClass = env->GetObjectClass(insets);
            const auto leftField = env->GetFieldID(insetsClass, "left", "I");
            const auto topField = env->GetFieldID(insetsClass, "top", "I");
            const auto rightField = env->GetFieldID(insetsClass, "right", "I");
            const auto bottomField = env->GetFieldID(insetsClass, "bottom", "I");

            const auto left = env->GetIntField(insets, leftField);
            const auto top = env->GetIntField(insets, topField);
            const auto right = env->GetIntField(insets, rightField);
            const auto bottom = env->GetIntField(insets, bottomField);

            env->DeleteLocalRef(insets);

            insets = getCaptionBarInsets(env, activity);

            const auto captionBarLeft = env->GetIntField(insets, leftField);
            const auto captionBarTop = env->GetIntField(insets, topField);
            const auto captionBarRight = env->GetIntField(insets, rightField);
            const auto captionBarBottom = env->GetIntField(insets, bottomField);

            env->DeleteLocalRef(insets);

            insets = getCaptionBarInsetsIgnoringVisibility(env, activity);

            const auto captionBarIgnoringVisibilityLeft = env->GetIntField(insets, leftField);
            const auto captionBarIgnoringVisibilityTop = env->GetIntField(insets, topField);
            const auto captionBarIgnoringVisibilityRight = env->GetIntField(insets, rightField);
            const auto captionBarIgnoringVisibilityBottom = env->GetIntField(insets, bottomField);

            env->DeleteLocalRef(insets);
            env->DeleteLocalRef(insetsClass);

            if (config::freeform_window_include_caption_bar_padding) {
                if (captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight > 0 &&
                    captionBarLeft + captionBarRight == 0) {
                    width -= captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight;
                }

                if (captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom > 0 &&
                    captionBarTop + captionBarBottom == 0) {
                    height -= captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom;
                }
            } else {
                if (captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight > 0) {
                    width -= captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight;
                }

                if (captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom > 0) {
                    height -= captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom;
                }
            }

            if (!isEdgeToEdgeEnabled(env, activity)) {
                width -= left + right;
                height -= top + bottom;
            }

            const auto isPortrait = width <= height;

            Gallop::Screen::OriginalScreenWidth(isPortrait ? height : width);
            Gallop::Screen::OriginalScreenHeight(isPortrait ? width : height);

            ResizeWindow(width, height);
        }

        if (!config::unlock_live_chara) {
            try {
                const auto path = il2cpp_symbols::get_method_pointer<Il2CppString *(*)()>(
                        "Cute.Core.Assembly.dll", "Cute.Core", "Device", "GetPersistentDataPath",
                        0)()->chars;

                if (filesystem::exists(
                        path + il2cppstring(IL2CPP_STRING(R"(\master\master_orig.mdb)")))) {
                    filesystem::remove_all(path + il2cppstring(IL2CPP_STRING(R"(\master)")));
                }
            } catch (const exception &ex) {
                LOGW("Failed to remove master_orig.mdb: %s", ex.what());
            }
        }

        StartTickFrame();

        auto sceneManagerClass = il2cpp_symbols::get_class("UnityEngine.CoreModule.dll",
                                                           "UnityEngine.SceneManagement",
                                                           "SceneManager");

        auto activeSceneChangedField = il2cpp_class_get_field_from_name(sceneManagerClass,
                                                                        "activeSceneChanged");

        auto action = CreateDelegateWithClassStatic(
                il2cpp_class_from_type(activeSceneChangedField->type),
                *([](void *, const UnityEngine::SceneManagement::Scene scene,
                     const UnityEngine::SceneManagement::Scene scene1) {

                    auto sceneManager = Gallop::SceneManager::Instance();

                    if (!sceneManager) {
                        return;
                    }

                    il2cppstring sceneName = sceneManager.GetCurrentSceneIdName()->chars;

                    const auto uiManager = Gallop::UIManager::Instance();

                    const auto env = GetJNIEnv();
                    const auto activity = GetActivity();

                    if (sceneName == IL2CPP_STRING("Live")) {
                        showMediaNotification(env, activity);

                        const auto loadSettings = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)()>(
                                "umamusume.dll", "Gallop.Live", "Director", "get_LoadSettings",
                                IgnoreNumberOfArguments)();
                        const auto musicId = il2cpp_class_get_method_from_name_type<int (*)(
                                Il2CppObject *)>(loadSettings->klass, "get_MusicId",
                                                 0)->methodPointer(loadSettings);

                        MediaNotificationManager::UpdateMetadata(musicId);
                        updateMediaPlayWhenReady(env, activity, false);
                        updateMediaNavigationButtons(env, activity, true, false);
                    } else if (sceneName == IL2CPP_STRING("Home")) {
                        showMediaNotification(env, activity);
                        updateMediaPlayWhenReady(env, activity, false);
                        updateMediaNavigationButtons(env, activity, false, false);
                        MediaNotificationManager::UpdateMetadata();
                    } else {
                        updateMediaPlayWhenReady(env, activity, false);
                        updateMediaNavigationButtons(env, activity, false, false);
                        hideNotification(env, activity);
                    }

                    if (sceneName == IL2CPP_STRING("Title")) {
                        if (config::character_system_text_caption) {
                            Localify::NotificationManager::Reset();
                        }

                        if (config::freeform_window) {
                            const auto width = UnityEngine::Screen::width();
                            const auto height = UnityEngine::Screen::height();

                            const auto isVirt = width < height;
                            Gallop::Screen::OriginalScreenWidth(width);
                            Gallop::Screen::OriginalScreenHeight(height);
                        }
                    }

                    if (sceneName == IL2CPP_STRING("Home")) {
                        if (config::character_system_text_caption) {
                            Localify::NotificationManager::Init();
                        }

                        if (config::unlock_live_chara) {
                            const auto charaList = MsgPackModify::GetCharaList();

                            const auto workDataManager = GetSingletonInstance(
                                    il2cpp_symbols::get_class("umamusume.dll", "Gallop",
                                                              "WorkDataManager"));

                            const auto workCharaData = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                    Il2CppObject *)>(workDataManager->klass, "get_CharaData", 0)(
                                    workDataManager);

                            auto UserCharaClass = il2cpp_symbols::get_class("umamusume.Http.dll",
                                                                            "Gallop", "UserChara");

                            if (!UserCharaClass) {
                                UserCharaClass = il2cpp_symbols::get_class("umamusume.dll",
                                                                           "Gallop", "UserChara");
                            }

                            for (const auto &chara: charaList) {
                                const auto userChara = il2cpp_object_new(UserCharaClass);

                                const auto chara_id_field = il2cpp_class_get_field_from_name(
                                        userChara->klass, "chara_id");
                                auto chara_id = chara["chara_id"].int32_value();
                                il2cpp_field_set_value(userChara, chara_id_field, &chara_id);

                                const auto training_num_field = il2cpp_class_get_field_from_name(
                                        userChara->klass, "training_num");
                                auto training_num = chara["training_num"].int32_value();
                                il2cpp_field_set_value(userChara, training_num_field,
                                                       &training_num);

                                const auto love_point_field = il2cpp_class_get_field_from_name(
                                        userChara->klass, "love_point");
                                auto love_point = chara["love_point"].int32_value();
                                il2cpp_field_set_value(userChara, love_point_field, &love_point);

                                const auto love_point_pool_field = il2cpp_class_get_field_from_name(
                                        userChara->klass, "love_point_pool");
                                if (love_point_pool_field) {
                                    auto love_point_pool = chara["love_point_pool"].int32_value();
                                    il2cpp_field_set_value(userChara, love_point_pool_field,
                                                           &love_point_pool);
                                }

                                const auto fan_field = il2cpp_class_get_field_from_name(userChara->klass,
                                                                                  "fan");
                                auto fan = chara["fan"].uint64_value();
                                il2cpp_field_set_value(userChara, fan_field, &fan);

                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            Il2CppObject *)>(
                                        workCharaData->klass, "UpdateCharaData", 1)(workCharaData,
                                                                                    userChara);
                            }
                        }
                    }

                    if (sceneName == IL2CPP_STRING("Live") && config::champions_live_show_text) {
                        const auto loadSettings = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)()>(
                                "umamusume.dll", "Gallop.Live", "Director", "get_LoadSettings",
                                IgnoreNumberOfArguments)();
                        const auto musicId = il2cpp_symbols::get_method_pointer<int (*)(Il2CppObject *)>(
                                loadSettings->klass, "get_MusicId", 0)(loadSettings);

                        if (musicId == 1054) {
                            const auto raceInfo = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                    Il2CppObject *)>(loadSettings->klass, "get_raceInfo", 0)(
                                    loadSettings);

                            const auto resourceId = il2cpp_symbols::get_method_pointer<int (*)(
                                    Il2CppObject *)>(raceInfo->klass,
                                                     "get_ChampionsMeetingResourceId", 0)(raceInfo);

                            if (resourceId == 0) {
                                const auto charaNameArray = il2cpp_array_new_type<Il2CppString *>(
                                        il2cpp_defaults.string_class, 9);
                                const auto trainerNameArray = il2cpp_array_new_type<Il2CppString *>(
                                        il2cpp_defaults.string_class, 9);

                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            Il2CppArraySize_t<Il2CppString *> *)>(
                                        raceInfo->klass, "set_CharacterNameArray", 1)(raceInfo,
                                                                                      charaNameArray);
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            Il2CppArraySize_t<Il2CppString *> *)>(
                                        raceInfo->klass, "set_TrainerNameArray", 1)(raceInfo,
                                                                                    trainerNameArray);

                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            Il2CppArraySize_t<Il2CppString *> *)>(
                                        raceInfo->klass, "set_CharacterNameArrayForChampionsText",
                                        1)(raceInfo, nullptr);
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *,
                                                                            Il2CppArraySize_t<Il2CppString *> *)>(
                                        raceInfo->klass, "set_TrainerNameArrayForChampionsText", 1)(
                                        raceInfo, nullptr);

                                const auto charaInfoList = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                        Il2CppObject *)>(loadSettings->klass,
                                                         "get_CharacterInfoList", 0)(loadSettings);

                                const auto itemsField = il2cpp_class_get_field_from_name(
                                        charaInfoList->klass, "_items");
                                Il2CppArraySize_t<Il2CppObject *> *charaInfoArr;
                                il2cpp_field_get_value(charaInfoList, itemsField, &charaInfoArr);

                                for (int i = 0; i < 9; i++) {
                                    const auto info = charaInfoArr->vector[i];
                                    const auto charaId = il2cpp_symbols::get_method_pointer<int (*)(
                                            Il2CppObject *)>(info->klass, "get_CharaId", 0)(info);
                                    const auto mobId = il2cpp_symbols::get_method_pointer<int (*)(
                                            Il2CppObject *)>(info->klass, "get_MobId", 0)(info);

                                    Il2CppString *charaName;
                                    if (charaId == 1) {
                                        charaName = il2cpp_symbols::get_method_pointer<Il2CppString *(*)(
                                                int, int)>("umamusume.dll", "Gallop", "TextUtil",
                                                           "GetMasterText", 2)(59, mobId);
                                    } else {
                                        charaName = il2cpp_symbols::get_method_pointer<Il2CppString *(*)(
                                                int, int)>("umamusume.dll", "Gallop", "TextUtil",
                                                           "GetMasterText", 2)(6, charaId);
                                    }

                                    il2cpp_array_setref(charaNameArray, i, charaName);
                                    il2cpp_array_setref(trainerNameArray, i, il2cpp_string_new(""));
                                }

                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, int)>(
                                        raceInfo->klass, "set_ChampionsMeetingResourceId", 1)(
                                        raceInfo, config::champions_live_resource_id);
                                il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, int)>(
                                        raceInfo->klass, "set_DateYear", 1)(raceInfo,
                                                                            config::champions_live_year);
                            }
                        }
                    }
                }));
        il2cpp_field_static_set_value(activeSceneChangedField, action);
    }

    static void *il2cpp_handle = nullptr;

    static bool dlopen_process(const char *name, void *handle) {
        if (!il2cpp_handle) {
            if (name != nullptr && strstr(name, "libil2cpp.so")) {
                il2cpp_handle = handle;
                LOGI("Got il2cpp handle: %p", handle);

                config::read_config_init();

                il2cpp_init_addr = dlsym(il2cpp_handle, "il2cpp_init");
                il2cpp_symbols::init(il2cpp_handle);
                DobbyHook(il2cpp_init_addr, reinterpret_cast<void *>(il2cpp_init_hook),
                          &il2cpp_init_orig);

                thread init_thread([] {
                    logger::init_logger();
                    local::load_textdb(&config::dicts);

                    if (!config::text_id_dict.empty()) {
                        local::load_textId_textdb(config::text_id_dict);
                    }
                });
                init_thread.detach();
                return true;
            }
        }
        return false;
    }

    HOOK_DEF(void*, do_dlopen, const char *name, int flags) {
        void *handle = orig_do_dlopen(name, flags);
        if (dlopen_process(name, handle)) {
            DobbyDestroy(addr_do_dlopen);
        }
        return handle;
    }

    HOOK_DEF(void*, do_dlopen_V24, const char *name, int flags,
             const void *extinfo [[maybe_unused]],
             void *caller_addr [[maybe_unused]]) {
        void *handle = orig_do_dlopen_V24(name, flags, extinfo, caller_addr);
        if (dlopen_process(name, handle)) {
            DobbyDestroy(addr_do_dlopen_V24);
        }
        return handle;
    }

    HOOK_DEF(void*, NativeBridgeLoadLibraryExt_V30, const char *filename, int flag,
             struct native_bridge_namespace_t *ns) {
        LOGD("NativeBridgeLoadLibraryExt_V30: %s", filename);
        if (string(filename).find(string("libmain.so")) != string::npos) {
            auto nativeBridge = dlopen("libnativebridge.so", RTLD_NOW);
            auto *NativeBridgeError = reinterpret_cast<bool (*)()>(dlsym(nativeBridge,
                                                                         "NativeBridgeError"));
            auto *NativeBridgeGetError = reinterpret_cast<char *(*)()>(dlsym(nativeBridge,
                                                                             "NativeBridgeGetError"));
            auto *NativeBridgeGetTrampoline = reinterpret_cast<void *(*)(void *handle,
                                                                         const char *name,
                                                                         const char *shorty,
                                                                         uint32_t len)>(dlsym(
                    nativeBridge, "NativeBridgeGetTrampoline"));

            stringstream path_armV8;
            path_armV8 << "/data/data/" << Game::GetCurrentPackageName().data() << "/arm64-v8a.so";
            stringstream path_armV7;
            path_armV7 << "/data/data/" << Game::GetCurrentPackageName().data()
                       << "/armeabi-v7a.so";

            string path;

            if (access(path_armV8.str().data(), F_OK) != -1) {
                path = path_armV8.str();
            } else if (access(path_armV7.str().data(), F_OK) != -1) {
                path = path_armV7.str();
            }

            if (!path.empty()) {
                void *lib = orig_NativeBridgeLoadLibraryExt_V30(path.data(), RTLD_NOW, ns);
                if (NativeBridgeError()) {
                    if (auto error_bridge = NativeBridgeGetError()) {
                        LOGW("error_bridge: %s", error_bridge);
                    }
                }

                auto hook = reinterpret_cast<void (*)(JNIEnv *,
                                                      Resource *)>(NativeBridgeGetTrampoline(
                        lib, "hook", "VLL", 3));
                hook(env, classesDex);

                DobbyDestroy(addr_NativeBridgeLoadLibraryExt_V30);
            }
        }

        return orig_NativeBridgeLoadLibraryExt_V30(filename, flag, ns);
    }

    HOOK_DEF(void*, NativeBridgeLoadLibraryExt_V26, const char *filename, int flag,
             struct native_bridge_namespace_t *ns) {
        if (string(filename).find(string("libmain.so")) != string::npos) {
            auto nativeBridge = dlopen("libnativebridge.so", RTLD_NOW);
            auto *NativeBridgeError = reinterpret_cast<bool (*)()>(dlsym(nativeBridge,
                                                                         "_ZN7android17NativeBridgeErrorEv"));
            auto *NativeBridgeGetError = reinterpret_cast<char *(*)()>(dlsym(nativeBridge,
                                                                             "_ZN7android20NativeBridgeGetErrorEv"));
            auto *NativeBridgeGetTrampoline = reinterpret_cast<void *(*)(void *handle,
                                                                         const char *name,
                                                                         const char *shorty,
                                                                         uint32_t len)>(dlsym(
                    nativeBridge, "_ZN7android25NativeBridgeGetTrampolineEPvPKcS2_j"));

            stringstream path_armV8;
            path_armV8 << "/data/data/" << Game::GetCurrentPackageName().data() << "/arm64-v8a.so";
            stringstream path_armV7;
            path_armV7 << "/data/data/" << Game::GetCurrentPackageName().data()
                       << "/armeabi-v7a.so";

            string path;

            if (access(path_armV8.str().data(), F_OK) != -1) {
                path = path_armV8.str();
            } else if (access(path_armV7.str().data(), F_OK) != -1) {
                path = path_armV7.str();
            }

            if (!path.empty()) {
                void *lib = orig_NativeBridgeLoadLibraryExt_V26(path.data(), RTLD_NOW, ns);
                if (NativeBridgeError()) {
                    if (auto error_bridge = NativeBridgeGetError()) {
                        LOGW("error_bridge: %s", error_bridge);
                    }
                }

                auto hook = reinterpret_cast<void (*)(JNIEnv *,
                                                      Resource *)>(NativeBridgeGetTrampoline(
                        lib, "hook", "VLL", 3));
                hook(env, classesDex);
                DobbyDestroy(addr_NativeBridgeLoadLibraryExt_V26);
            }
        }

        return orig_NativeBridgeLoadLibraryExt_V26(filename, flag, ns);
    }

    HOOK_DEF(void*, NativeBridgeLoadLibrary_V21, const char *filename, int flag) {
        if (string(filename).find(string("libmain.so")) != string::npos) {
            auto nativeBridge = dlopen("libnativebridge.so", RTLD_NOW);
            auto *NativeBridgeError = reinterpret_cast<bool (*)()>(dlsym(nativeBridge,
                                                                         "_ZN7android17NativeBridgeErrorEv"));
            auto *NativeBridgeGetTrampoline = reinterpret_cast<void *(*)(void *handle,
                                                                         const char *name,
                                                                         const char *shorty,
                                                                         uint32_t len)>(dlsym(
                    nativeBridge, "_ZN7android25NativeBridgeGetTrampolineEPvPKcS2_j"));

            stringstream path_armV8;
            path_armV8 << "/data/data/" << Game::GetCurrentPackageName().data() << "/arm64-v8a.so";
            stringstream path_armV7;
            path_armV7 << "/data/data/" << Game::GetCurrentPackageName().data()
                       << "/armeabi-v7a.so";

            string path;

            if (access(path_armV8.str().data(), F_OK) != -1) {
                path = path_armV8.str();
            } else if (access(path_armV7.str().data(), F_OK) != -1) {
                path = path_armV7.str();
            }

            if (!path.empty()) {
                void *lib = orig_NativeBridgeLoadLibrary_V21(path.data(), RTLD_NOW);
                if (NativeBridgeError()) {
                    LOGW("LoadLibrary failed");
                }

                auto hook = reinterpret_cast<void (*)(JNIEnv *,
                                                      Resource *)>(NativeBridgeGetTrampoline(
                        lib, "hook", "VLL", 3));
                hook(env, classesDex);
                DobbyDestroy(addr_NativeBridgeLoadLibrary_V21);
            }
        }

        return orig_NativeBridgeLoadLibrary_V21(filename, flag);
    }
}

extern "C" void
onLayoutChange_native(JNIEnv *env, jclass /*clazz*/, jobject activity, jobject /*view*/,
                      jint /*left*/,
                      jint /*top*/, jint /*right*/, jint /*bottom*/, jint /*oldLeft*/,
                      jint /*oldTop*/, jint /*oldRight*/, jint /*oldBottom*/) {
    if (!config::freeform_window) {
        return;
    }

    if (IsABIRequiredNativeBridge()) {
        return;
    }

    if (!il2cpp_is_vm_thread || !il2cpp_is_vm_thread(il2cpp_thread_current())) {
        return;
    }

    const auto windowMetricsCalculatorClass = env->GetObjectClass(windowMetricsCalculator);

    const auto computeId = env->GetMethodID(windowMetricsCalculatorClass,
                                      "computeCurrentWindowMetrics",
                                      "(Landroid/app/Activity;)Landroidx/window/layout/WindowMetrics;");
    const auto metrics = env->CallObjectMethod(windowMetricsCalculator, computeId, activity);

    const auto metricsClass = env->GetObjectClass(metrics);

    const auto getRectId = env->GetMethodID(metricsClass, "getBounds", "()Landroid/graphics/Rect;");
    const auto rect = env->CallObjectMethod(metrics, getRectId);

    env->DeleteLocalRef(metrics);
    env->DeleteLocalRef(metricsClass);

    const auto rectClass = env->GetObjectClass(rect);

    const auto widthId = env->GetMethodID(rectClass, "width", "()I");
    jint width = env->CallIntMethod(rect, widthId);

    const auto heightId = env->GetMethodID(rectClass, "height", "()I");
    jint height = env->CallIntMethod(rect, heightId);

    env->DeleteLocalRef(rect);
    env->DeleteLocalRef(rectClass);

    auto insets = getDisplayCutoutInsets(env, activity);
    auto insetsClass = env->GetObjectClass(insets);
    auto leftField = env->GetFieldID(insetsClass, "left", "I");
    auto topField = env->GetFieldID(insetsClass, "top", "I");
    auto rightField = env->GetFieldID(insetsClass, "right", "I");
    auto bottomField = env->GetFieldID(insetsClass, "bottom", "I");

    const auto left = env->GetIntField(insets, leftField);
    const auto top = env->GetIntField(insets, topField);
    const auto right = env->GetIntField(insets, rightField);
    const auto bottom = env->GetIntField(insets, bottomField);

    env->DeleteLocalRef(insets);

    insets = getCaptionBarInsets(env, activity);

    const auto captionBarLeft = env->GetIntField(insets, leftField);
    const auto captionBarTop = env->GetIntField(insets, topField);
    const auto captionBarRight = env->GetIntField(insets, rightField);
    const auto captionBarBottom = env->GetIntField(insets, bottomField);

    env->DeleteLocalRef(insets);

    insets = getCaptionBarInsetsIgnoringVisibility(env, activity);

    const auto captionBarIgnoringVisibilityLeft = env->GetIntField(insets, leftField);
    const auto captionBarIgnoringVisibilityTop = env->GetIntField(insets, topField);
    const auto captionBarIgnoringVisibilityRight = env->GetIntField(insets, rightField);
    const auto captionBarIgnoringVisibilityBottom = env->GetIntField(insets, bottomField);

    env->DeleteLocalRef(insets);
    env->DeleteLocalRef(insetsClass);

    if (config::freeform_window_include_caption_bar_padding) {
        if (captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight > 0 &&
            captionBarLeft + captionBarRight == 0) {
            width -= captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight;
        }

        if (captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom > 0 &&
            captionBarTop + captionBarBottom == 0) {
            height -= captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom;
        }
    } else {
        if (captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight > 0) {
            width -= captionBarIgnoringVisibilityLeft + captionBarIgnoringVisibilityRight;
        }

        if (captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom > 0) {
            height -= captionBarIgnoringVisibilityTop + captionBarIgnoringVisibilityBottom;
        }
    }

    if (!isEdgeToEdgeEnabled(env, activity)) {
        width -= left + right;
        height -= top + bottom;
    }

    const auto gameSystem = Gallop::GameSystem::Instance();

    const auto ValueTuple2Class = GetGenericClass(
            GetRuntimeType("mscorlib.dll", "System", "ValueTuple`2"),
            GetRuntimeType(il2cpp_defaults.int32_class),
            GetRuntimeType(il2cpp_defaults.int32_class));
    auto tuple = System::ValueTuple<int, int>{width, height};
    const auto boxed = il2cpp_value_box(ValueTuple2Class, &tuple);

    const auto fn = *[](Il2CppObject *self) {
        const auto tuple = *il2cpp_object_unbox_type<System::ValueTuple<int, int> *>(self);
        ResizeWindow(tuple.Item1, tuple.Item2);
    };
    il2cpp_symbols::get_method_pointer<void (*)(Il2CppObject *, Il2CppDelegate *)>(
            "umamusume.dll",
            "Gallop",
            "MonoBehaviourExtension",
            "WaitForEndFrame",
            2)(gameSystem,
               CreateDelegate(
                       boxed,
                       fn));
}

extern "C" void
handleSetPlayWhenReady_native(JNIEnv *env, jclass /*clazz*/, jboolean playWhenReady) {
    if (playWhenReady) {
        WaitForEndOfFrame(*[] {
            const auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();
            const auto hubViewController = GetCurrentHubViewChildController();

            if (hubViewController && hubViewController->klass->name == "HomeViewController"s) {
                const auto topUi = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *, int)>(hubViewController->klass, "GetTopUI",
                                              1)->methodPointer(hubViewController, 10);
                if (topUi) {
                    const auto data = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                            Il2CppObject *)>(topUi->klass, "get_TempSetListPlayingData",
                                             0)->methodPointer(topUi);

                    const auto SetListIdField = il2cpp_class_get_field_from_name(data->klass,
                                                                           "SetListId");
                    int SetListId;
                    il2cpp_field_get_value(data, SetListIdField, &SetListId);

                    if (SetListId > 0) {
                        Il2CppObject *_jukeboxBgmSelector = il2cpp_symbols::get_method_pointer<Il2CppObject *(*)(
                                Il2CppObject *)>(topUi->klass, "get_JukeboxBgmSelector", 0)(topUi);
                        il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *, bool, float,
                                                                        bool)>(
                                _jukeboxBgmSelector->klass, "PlayCoroutinePlaySetList",
                                3)->methodPointer(_jukeboxBgmSelector, true, 0.0f, true);
                    } else {
                        il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *)>(
                                topUi->klass, "PlayRequestSong", 0)->methodPointer(topUi);
                    }
                }
            }

            if (controller && controller->klass->name == "LiveViewController"s) {
                il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *)>(controller->klass,
                                                                                 "ResumeLive",
                                                                                 0)->methodPointer(
                        controller);
            }
        });
    } else {
        WaitForEndOfFrame(*[] {
            const auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();
            const auto hubViewController = GetCurrentHubViewChildController();

            if (hubViewController && hubViewController->klass->name == "HomeViewController"s) {
                if (const auto topUi = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *, int)>(hubViewController->klass, "GetTopUI",
                                              1)->methodPointer(hubViewController, 10)) {
                    il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *, bool)>(
                            topUi->klass, "SetPlayMusicFlag", 1)->methodPointer(topUi, false);
                }
            }

            if (controller && controller->klass->name == "LiveViewController"s) {
                il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *)>(controller->klass,
                                                                                 "PauseLive",
                                                                                 0)->methodPointer(
                        controller);
            }
        });
    }
}

extern "C" void
handleSeek_native(JNIEnv *env, jclass /*clazz*/, jint mediaItemIndex, jlong positionMs,
                  jint seekCommand) {
    if (seekCommand == 5) {
        Localify::LiveUtils::MoveLivePlayback(static_cast<float>(positionMs) / 1000);
    }

    if (seekCommand == 7) {
        WaitForEndOfFrame(*[] {
            const auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();
            const auto hubViewController = GetCurrentHubViewChildController();

            if (hubViewController && hubViewController->klass->name == "HomeViewController"s) {
                const auto topUi = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *, int)>(hubViewController->klass, "GetTopUI",
                                              1)->methodPointer(hubViewController, 10);
                il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *, bool)>(topUi->klass,
                                                                                       "OnClickSetListArrow",
                                                                                       1)->methodPointer(
                        topUi, false);
            }
        });
    }

    if (seekCommand == 9) {
        WaitForEndOfFrame(*[] {
            const auto controller = Gallop::SceneManager::Instance().GetCurrentViewController();
            const auto hubViewController = GetCurrentHubViewChildController();

            if (hubViewController && hubViewController->klass->name == "HomeViewController"s) {
                const auto topUi = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *, int)>(hubViewController->klass, "GetTopUI",
                                              1)->methodPointer(hubViewController, 10);

                const auto data = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *)>(topUi->klass, "get_TempSetListPlayingData",
                                         0)->methodPointer(topUi);

                const auto IsPlayingField = il2cpp_class_get_field_from_name(data->klass, "IsPlaying");
                bool IsPlaying;
                il2cpp_field_get_value(data, IsPlayingField, &IsPlaying);

                if (IsPlaying) {
                    il2cpp_class_get_method_from_name_type<void (*)(Il2CppObject *, bool)>(
                            topUi->klass, "OnClickSetListArrow", 1)->methodPointer(topUi, true);
                }
            }

            if (controller && controller->klass->name == "LiveViewController"s) {
                const auto view = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *)>(controller->klass, "GetViewBase", 0)->methodPointer(
                        controller);
                const auto coroutine = il2cpp_class_get_method_from_name_type<Il2CppObject *(*)(
                        Il2CppObject *)>(controller->klass, "SkipLive", 0)->methodPointer(
                        controller);

                UnityEngine::MonoBehaviour(view).StartCoroutineManaged2(coroutine);
            }
        });
    }
}

void hack_thread(const HookArgs *args) {
    LOGI("%s hack thread: %d", ABI, gettid());

    const int api_level = GetAndroidApiLevel();
    LOGI("%s api level: %d", ABI, api_level);

    env = args->env;
    classesDex = args->classesDex;

    void *addr = nullptr;
    if (IsRunningOnNativeBridge()) {
        addr = reinterpret_cast<void *>(dlopen);
    } else if (!IsABIRequiredNativeBridge()) {
        if (ABI == "x86"s) {
            addr = reinterpret_cast<void *>(dlopen);
        } else {
            addr = DobbySymbolResolver(nullptr,
                                       "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
        }
    }

    if (addr) {
        LOGI("%s do_dlopen at: %p", ABI, addr);
        if (IsRunningOnNativeBridge() || ABI == "x86"s) {
            addr_do_dlopen = addr;
            DobbyHook(addr_do_dlopen, reinterpret_cast<void *>(new_do_dlopen),
                      reinterpret_cast<void **>(&orig_do_dlopen));
        } else {
            addr_do_dlopen_V24 = addr;
            DobbyHook(addr_do_dlopen_V24, reinterpret_cast<void *>(new_do_dlopen_V24),
                      reinterpret_cast<void **>(&orig_do_dlopen_V24));
        }
    }

    if (IsABIRequiredNativeBridge()) {
        if (api_level >= 30) {
            addr_NativeBridgeLoadLibraryExt_V30 = dlsym(dlopen("libnativebridge.so", RTLD_NOW),
                                                        "NativeBridgeLoadLibraryExt");
            if (addr_NativeBridgeLoadLibraryExt_V30) {
                LOGI("NativeBridgeLoadLibraryExt at: %p", addr_NativeBridgeLoadLibraryExt_V30);
                DobbyHook(addr_NativeBridgeLoadLibraryExt_V30,
                          reinterpret_cast<void *>(new_NativeBridgeLoadLibraryExt_V30),
                          reinterpret_cast<void **>(&orig_NativeBridgeLoadLibraryExt_V30));
            }
        } else if (api_level >= 26) {
            addr_NativeBridgeLoadLibraryExt_V26 = DobbySymbolResolver(nullptr,
                                                                      "_ZN7android26NativeBridgeLoadLibraryExtEPKciPNS_25native_bridge_namespace_tE");
            if (addr_NativeBridgeLoadLibraryExt_V26) {
                LOGI("NativeBridgeLoadLibraryExt at: %p", addr_NativeBridgeLoadLibraryExt_V26);
                DobbyHook(addr_NativeBridgeLoadLibraryExt_V26,
                          reinterpret_cast<void *>(new_NativeBridgeLoadLibraryExt_V26),
                          reinterpret_cast<void **>(&orig_NativeBridgeLoadLibraryExt_V26));
            }
        } else {
            addr_NativeBridgeLoadLibrary_V21 = DobbySymbolResolver(nullptr,
                                                                   "_ZN7android23NativeBridgeLoadLibraryEPKci");
            if (addr_NativeBridgeLoadLibrary_V21) {
                LOGI("NativeBridgeLoadLibrary at: %p", addr_NativeBridgeLoadLibrary_V21);
                DobbyHook(addr_NativeBridgeLoadLibrary_V21,
                          reinterpret_cast<void *>(new_NativeBridgeLoadLibrary_V21),
                          reinterpret_cast<void **>(&orig_NativeBridgeLoadLibrary_V21));
            }
        }
    }
}
