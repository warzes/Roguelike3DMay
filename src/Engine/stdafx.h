#pragma once

#include "BaseHeader.h"

// TODO: тут внутренние заголовки

#if defined(_MSC_VER)
// TODO: потом убрать и исправить ошибки

#	pragma warning(disable : 4061) // не все перечисления обработаны - возможно пофиксить и убрать
#	pragma warning(disable : 4514) // подставляемая функция, не используемая в ссылках, была удалена
#	pragma warning(disable : 4623) // конструктор по умолчанию неявно определен как удаленный
#	pragma warning(disable : 4625) // конструктор копий неявно определен как удаленный
#	pragma warning(disable : 4626) // оператор назначения неявно определен как удаленный
#	pragma warning(disable : 4820) // добавление байт для выравнивания
#	pragma warning(disable : 5027) // оператор назначения перемещением неявно определен как удаленный
#	pragma warning(disable : 5045) // Компилятор вставит компонент устранения рисков Spectre

#endif