#include "SerializerInterface.h"
#include "nanoid/nanoid.h"
using namespace osgVerse;

static int g_headerFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Bullet
                         | ImGuiTreeNodeFlags_OpenOnDoubleClick;

SerializerInterface::SerializerInterface(osg::Object* obj, LibraryEntry* entry,
                                         const LibraryEntry::Property& prop, bool composited)
    : _object(obj), _entry(entry), _property(prop), _indent(2.0f), _selected(false), _dirty(true)
{ _postfix = "##" + nanoid::generate(8); _composited = composited; }

bool SerializerInterface::show(ImGuiManager* mgr, ImGuiContentHandler* content)
{
    bool toOpen = true; ImGui::Indent(_indent);
    if (_composited)
    {
        std::string title = TR(_property.name) + _postfix;
        if (ImGui::ArrowButton((title + "_Arrow").c_str(), ImGuiDir_Down))  // TODO: disabled = ImGuiDir_None
        {
            // Select the item and also open popup menu
            ImGui::OpenPopup((title + "_Popup").c_str());
        }
        ImGui::SameLine();

        if (_selected) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.4f, 0.0f, 1.0f));
        toOpen = ImGui::CollapsingHeader(title.c_str(), g_headerFlags);
        if (_selected) ImGui::PopStyleColor();

        if (ImGui::BeginPopup((title + "_Popup").c_str()))
        {
            // TODO: up/down/edit/delete for custom interface
            ImGui::EndPopup();
        }
    }

    if (toOpen) { toOpen = showProperty(mgr, content); _dirty = false; }
    ImGui::Unindent(_indent); return toOpen;
}

SerializerFactory* SerializerFactory::instance()
{
    static osg::ref_ptr<SerializerFactory> s_instance = new SerializerFactory;
    return s_instance.get();
}

LibraryEntry* SerializerFactory::createInterfaces(osg::Object* obj, LibraryEntry* lastEntry,
                                                  std::vector<osg::ref_ptr<SerializerInterface>>& interfaces)
{
    std::string libName = obj->libraryName(), clsName = obj->className();
    LibraryEntry* entry = (lastEntry && lastEntry->getLibraryName() == libName)
                        ? lastEntry : new LibraryEntry(libName);
    interfaces.clear();

    std::vector<LibraryEntry::Property> props = entry->getPropertyNames(clsName);
    SerializerFactory* factory = SerializerFactory::instance();
    for (size_t i = 0; i < props.size(); ++i)
    {
        osg::ref_ptr<SerializerInterface> si = factory->createInterface(obj, entry, props[i]);
        if (si.valid()) interfaces.push_back(si);
    }
    return entry;
}

SerializerInterface* SerializerFactory::createInterface(osg::Object* obj, LibraryEntry* entry,
                                                        const LibraryEntry::Property& prop)
{
    osgDB::BaseSerializer::Type t = prop.type;
    if (obj == NULL || entry == NULL)
    {
        OSG_WARN << "[SerializerFactory] Empty input arguments for " << prop.name << std::endl;
        return NULL;
    }

    if (_creatorMap.find(t) == _creatorMap.end())
    {
        OSG_WARN << "[SerializerFactory] Interface not implemented for " << prop.ownerClass
                 << "::" << prop.name << std::endl; return NULL;
    }
    return _creatorMap[t](obj, entry, prop);
}