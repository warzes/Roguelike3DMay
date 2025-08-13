#pragma once

#include "Engine/Engine.h"

#include <execution>
#include <numbers>
#include <iostream>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <ktx/ktx.h>

#include "OpenGL4Simple.h"
#include "ModelOLD.h"
#include "DepthPrepass.h"
#include "LightOLD.h"
#include "GraphicSystem.h"
#include "PipelineBloom.h"
#include "PipelineDeferredSSAO.h"
#include "PipelineShadowMapping.h"