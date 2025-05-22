# 03.05.2025
- Старт
- Проект Visual Studio из двух библиотек и приложения
- Дефолтные настройки проекта
- Зависимости для проекта: assimp, glad, glfw, glm, imgui, stb, 
- Глобальный лог
- Глобальная функция ExitApp() - функция для завершения приложения, эмулирующая выход по исключению или exit()
- Основной класс EngineApp управляющий циклом игры
- Создание и вывод окна

# 05.05.2025
- ресайз окна
- инициализация imGui
- profiler
- события glfw
- несколько общих функций
- события клавиатуры/мыши
- Render: создание 2D текстуры, шейдеров, буфера, вао, фреймбуфер
- Camera

# 06.05.2025
- Mesh
- Model
- загрузка моделей с assimp
- упрощенная CreateBuffer() принимающая вектор
- рабочие методы вывода куба из GraphicSystem
- тесты

# 07.05.2025
- Light
- В график добавлен вывод Quad. фикс вывода сферы
- PipelineShadowMapping - объект позволяющий рендерить карту теней от источника света
- Тест shadow map
- SetUniform принимающие строку. Не использовать в игре

# 11.05.2025
- PipelineDeferredSSAO для рендера Deffered Shading с SSAO

# 14.05.2025
- gl4::GetUniformBlockIndex и тест Uniform Buffer
- тест SSBO
- CreateShaderProgram для вычислительного шейдера
- первый вариант Forward+ shading для игры

# 15.05.2025 - 19.05.2025 - разработка Forward+ shading для игры
- DepthPrepass - класс рендерящий в буфер глубины
- CreateBuffer переименован в CreateBufferStorage так как создает буфер неизменяемого размера
- новый CreateBuffer
- папка data разделена на три: ExampleData, CoreData, GameData
- новая версия Forward+ shading

# 19.05.2025
- типобезопасные типы для OpenGL объектов. Дальше будут использоваться они, а не GLuint

# 21.05.2025
- поддержка создания шейдеров на основе кода spirv
- правки SetUniform
- новые методы для BufferId
- SamplerId и RenderBufferId
- другие правки OpenGL4Simple

# 22.05.2025
- базовые перечисления OpenGL
- Format
- новый вариант создание VAO с кешированием
- новый BufferStorageId