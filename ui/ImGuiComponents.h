#ifndef MANA_UI_IMGUICOMPONENTS_HPP
#define MANA_UI_IMGUICOMPONENTS_HPP

# define IMGUI_DEFINE_MATH_OPERATORS
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include "ImGui.h"
#include <string>
#include <map>

namespace osgVerse
{
    /// Variable 'name' is required as unique, may add '##' to hide the display
    /** Components:
        - Window
        - Label
        - Button
        - ImageButton
        - CheckBox
        - RadioButtonGroup
        - InputField
        - InputValueField
        - InputVectorField (V2, V3, V4, Color)
        - Slider (H, V, Knob)
        - MenuBar
        - ListView
        - TreeView
        - Timeline
        - CollapsingHeader: ImGui::CollapsingHeader()
        - Tab: ImGui::BeginTabBar(), ImGui::BeginTabItem(), ImGui::EndTabItem(), ImGui::EndTabBar()
        - Popup: ImGui::BeginPopup(), ImGui::EndPopup(); ImGui::OpenPopup()
        - Table: ImGui::BeginTable(), ImGui::TableNextRow(), ImGui::TableNextColumn(), ImGui::EndTable()

        TODO
        // ImSequencer: make it usable
        // Code editor
        // Node editor
        // ImGuiFileDialog utf8 problem...
        // Input component need IME
        // Multi-language support
     */

    struct ImGuiComponentBase : public osg::Referenced
    {
        typedef void (*ActionCallback)(ImGuiManager*, ImGuiContentHandler*, ImGuiComponentBase*);
        typedef void (*ActionCallback2)(ImGuiManager*, ImGuiContentHandler*,
                                        ImGuiComponentBase*, const std::string&);
        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content) = 0;
        virtual void showEnd() { /* nothing to do by default */ }
        virtual void showTooltip(const std::string& desc, float wrapPos = 10.0f);
        std::string TR(const std::string& s);  // multi-language support
        osg::ref_ptr<osg::Referenced> userData;

        static void setWidth(float width, bool fromLeft = true);
        static void adjustLine(bool newLine, bool sep = false, float indentX = 0.0f, float indentY = 0.0f);
        static void openFileDialog(const std::string& name, const std::string& title,
                                   const std::string& dir = ".", const std::string& filters=".*");
        static bool showFileDialog(const std::string& name, std::string& result);
    };

    struct Window : public ImGuiComponentBase
    {
        std::string name; float alpha;
        osg::Vec2 pos, pivot, sizeMin, sizeMax;
        bool isOpen, collapsed, useMenuBar;
        ImGuiWindowFlags flags;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        virtual void showEnd() { ImGui::End(); }
        Window(const std::string& n)
        :   name(n), alpha(1.0f), isOpen(true),
            collapsed(false), useMenuBar(false), flags(0) {}
    };

    struct Label : public ImGuiComponentBase
    {
        struct TextData
        {
            bool useBullet, disabled, wrapped; osg::Vec3 color;
            TextData() : useBullet(false), disabled(false), wrapped(true) {}
        };
        std::vector<TextData> attributes;
        std::vector<std::string> texts;
        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
    };

    struct Button : public ImGuiComponentBase
    {
        std::string name, tooltip;
        osg::Vec2 size; bool repeatable, styled;
        ImColor styleNormal, styleHovered, styleActive;
        ActionCallback callback;
        
        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        Button(const std::string& n)
            : name(n), repeatable(false), styled(false), callback(NULL) {}
    };

    struct ImageButton : public ImGuiComponentBase
    {
        std::string name, tooltip;
        osg::Vec2 size, uv0, uv1; bool imageOnly;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        ImageButton(const std::string& n)
            : name(n), uv1(1.0f, 1.0f), imageOnly(false), callback(NULL) {}
    };

    struct CheckBox : public ImGuiComponentBase
    {
        std::string name, tooltip; uint32_t value;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        CheckBox(const std::string& n, uint32_t v) : name(n), value(v), callback(NULL) {}
    };

    struct ComboBox : public ImGuiComponentBase
    {
        std::vector<std::string> items;
        std::string name, tooltip; int index;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        ComboBox(const std::string& n) : name(n), index(0), callback(NULL) {}
    };

    struct RadioButtonGroup : public ImGuiComponentBase
    {
        struct RadioData { std::string name, tooltip; };
        std::vector<RadioData> buttons;
        int value; bool inSameLine;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        RadioButtonGroup() : value(0), inSameLine(true), callback(NULL) {}
    };

    struct InputField : public ImGuiComponentBase
    {
        std::string name, value, tooltip, placeholder;
        ImGuiInputTextFlags flags;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        InputField(const std::string& n) : name(n), flags(0), callback(NULL) {}
    };

    struct InputValueField : public ImGuiComponentBase
    {
        enum Type { IntValue, FloatValue, DoubleValue } type;
        std::string name, format, tooltip;
        double value, minValue, maxValue, step;
        ImGuiInputTextFlags flags;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        InputValueField(const std::string& n)
        :   type(DoubleValue), name(n), value(0),
            minValue(0), maxValue(0), step(1), flags(0), callback(NULL) {}
    };

    struct InputVectorField : public InputValueField
    {
        osg::Vec4d vecValue;
        int vecNumber; bool asColor;
        // flags = ImGuiColorEditFlags

        template<typename T> void setVector(const T& vec) const
        {
            int num = std::min<int>(vec.num_components, vecNumber);
            for (int i = 0; i < num; ++i) vecValue[i] = (double)vec[i];
        }

        template<typename T> void getVector(T& vec) const
        {
            int num = std::min<int>(vec.num_components, vecNumber);
            for (int i = 0; i < num; ++i) vec[i] = (T::value_type)vecValue[i];
        }

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        InputVectorField(const std::string& n)
            : InputValueField(n), vecNumber(4), asColor(false) { step = 0; }
    };

    struct Slider : public ImGuiComponentBase
    {
        enum Type { IntValue, FloatValue } type;
        enum Shape { Horizontal, Vertical, Knob } shape;
        std::string name, format, tooltip;
        double value, minValue, maxValue; osg::Vec2 size;
        ImGuiSliderFlags flags;  // or ImGuiKnobFlags
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        Slider(const std::string& n)
        :   type(IntValue), shape(Horizontal), name(n), value(0), minValue(0), maxValue(100),
            size(24, 120), flags(0), callback(NULL) {}
    };

    struct MenuBar : public ImGuiComponentBase
    {
        struct MenuItemData
        {
            bool enabled, selected, checkable;
            std::string name, shortcut, tooltip;
            ActionCallback callback;
            MenuItemData(const std::string& n)
            :   name(n), enabled(true), selected(false),
                checkable(false), callback(NULL) {}
        };

        struct MenuData
        {
            bool enabled; std::string name;
            std::vector<MenuItemData> items;
            MenuData(const std::string& n) : name(n), enabled(true) {}
        };

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        std::vector<MenuData> menuDataList;
    };

    struct ListView : public ImGuiComponentBase
    {
        struct ListData
        {
            std::string name;
            osg::ref_ptr<osg::Referenced> userData;
        };
        std::vector<ListData> items;
        std::string name, tooltip;
        int index, rows;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        ListView(const std::string& n) : name(n), index(0), rows(5), callback(NULL) {}
    };

    struct TreeView : public ImGuiComponentBase
    {
        struct TreeData
        {
            ImGuiTreeNodeFlags flags;
            std::string id, name, tooltip;
            std::vector<TreeData> children;
            osg::ref_ptr<osg::Referenced> userData;
            TreeData() : flags(ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf) {}
        };
        std::vector<TreeData> treeDataList;
        std::string selectedItemID;
        ActionCallback2 callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        void showRecursively(TreeData& td, ImGuiManager*, ImGuiContentHandler*);
        TreeView() : callback(NULL) {}
    };

    struct Timeline : public ImGuiComponentBase
    {
        struct SequenceItem
        {
            osg::Vec2i range;
            int type; bool expanded;
            std::string name;
            osg::ref_ptr<osg::Referenced> userData;

            SequenceItem() : type(0), expanded(false) {}
            SequenceItem(const std::string& n, int t, int s, int e)
                : range(s, e), type(t), expanded(false), name(n) {}
        };
        std::vector<SequenceItem> items;

        osg::Vec2i frameRange;
        int firstFrame, currentFrame, selectedIndex, flags;
        bool expanded; void* seqInterface;
        ActionCallback callback;

        virtual bool show(ImGuiManager* mgr, ImGuiContentHandler* content);
        Timeline()
        :   frameRange(0, 100), firstFrame(0), currentFrame(0), selectedIndex(-1),
            flags(0), expanded(true), seqInterface(NULL), callback(NULL) {}
    };
}

#endif