// copyright 2014, Shannon Mackey <mackey@BUILDlab.net>
#ifndef JSONDATAINTERFACE_H
#define JSONDATAINTERFACE_H

//for JSONDataObject and JSONDataInterface
#include <string>
#include <list>
#include <map>
#include <array>
#include <memory>

//for old IDF text import only
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <random>

const std::string ACCEPTED_VERSION = "8.2";
//end old IDF text import only

extern "C"
{
#include "../../third_party/cJSON/cJSON.h"
}

namespace idfx
{

class IDDxField
{
public:
    IDDxField(std::string name);
    ~IDDxField();

    std::string name() {
        return _name;
    }
    void insertFieldProperties(std::string key, std::string value);
    std::string value(std::string property_type);
    uint32_t iddIndex();

private:
    std::string _name;
    std::map< std::string, std::string> *_field_properties;
//  Version Id      type       string
//                 idd_order       1
};


class IDDxObject
{
public:
    IDDxObject(cJSON *obj);
    ~IDDxObject();

    std::string getType() {
        return _my_type;
    }

    std::string fieldValue(std::string field_name, std::string property_type);
    std::string propertyValue(std::string property_name);
    std::string idd_field(uint32_t field_index);
    std::vector<std::string> orderedFieldNames();

    bool isValid();

private:
    std::string _my_type;
    std::shared_ptr< std::map<std::string, std::shared_ptr< IDDxField > > > _fields;
    std::shared_ptr< std::map<std::string, std::string > > _object_properties;


    void insertField(std::shared_ptr< idfx::IDDxField > iddx_field);

//   Version    Version Id
};

class IDDxObjects
{
public:
    IDDxObjects(const std::string &json_content);
    ~IDDxObjects();

    std::string getIDDxObjectFieldPropertyValue(std::string iddx_object_type, std::string field_name, std::string property_type);
    std::string getIDDxObjectPropertyValue(std::string iddx_object_type, std::string property_type);
  //  bool getIDDxObject(std::string iddx_object_type, std::shared_ptr< idfx::IDDxObject >& iddx_object);
    std::shared_ptr<IDDxObject> getIDDxObjectPtr(std::string iddx_object_type);

private:
    std::shared_ptr< std::map<std::string, std::shared_ptr< IDDxObject > > > _iddx_object_map;

    bool loadIDDxObjects(cJSON *schema_root);
    void insertIDDxObject(std::shared_ptr<IDDxObject> iddx_object);

    //           version
};

///////////////////// IDFxObject /////////////////////////////////////


class IDFxObject
{
public:
    IDFxObject(const std::string &json_content, std::shared_ptr<idfx::IDDxObjects> schema_objects);
    ~IDFxObject();


    std::string print();
//   std::list< std::string > dataIDF();

    std::string objectType() {
        return _object_type;
    }

//     uint32_t extensionCount() {
//       return _extensions->size();
//     }

    uint32_t propertyCount() {
        return _properties->size();
    }

    std::string value(std::string field_name);
    std::string value(u_int32_t field_index);



private:
    std::string _id;
    std::string _object_type;

    std::shared_ptr<IDDxObject> _schema_object;
    std::map<std::string, std::string> *_properties;
    std::vector<std::shared_ptr<IDFxObject> > *_extensions;

    void setProperties(cJSON *cjson_object);
    void setExtensions(cJSON *cjson_object, std::string extension_type, std::shared_ptr< idfx::IDDxObjects > schema_objects);
};


class IDFxObjects
{
public:
    IDFxObjects();//const std::string &json_content);
    ~IDFxObjects();

    void insertIDFxObject(std::shared_ptr<IDFxObject> idfx_object);
    std::shared_ptr< std::vector<std::shared_ptr< IDFxObject > > > objectVector() {
        return _idfx_objects;
    }

private:
    std::shared_ptr< std::vector<std::shared_ptr< IDFxObject > > > _idfx_objects;
};


///////////////////// JSONDataInterface /////////////////////////////////////

class JSONDataInterface
{
public:
    JSONDataInterface(const std::string &json_schema);
    ~JSONDataInterface();

    std::map<std::string, std::shared_ptr<IDFxObject> > getModelObjects(std::string object_type);

    bool exportIDFfile(std::string filename);

    bool importIDFxModel(const std::string &json_content);
    bool validateModel();
    void writeJSONdata(const std::string &filename);

    void importIDFxFile(std::string filename);
    void importIDFFile(std::string filename);


private:
    std::shared_ptr<IDDxObjects> _schema_objects;
    std::shared_ptr<IDFxObjects> _model_objects;

    bool loadIDDxObjects(cJSON *schema_root);


    void checkRange(cJSON *attribute,
                    const std::string &property_name,
                    const std::string &child_name,
                    bool &valid,
                    double property_value);
    void checkNumeric(double property_value,
                      const std::string &property_name,
                      cJSON *schema_object,
                      bool &valid,
                      const std::string &child_name);
};

} //idfxt namespace
#endif // JSONDATAINTERFACE_H