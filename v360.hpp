#include <Python.h>

#include <string>
#include <iostream>
#include <exception>

#ifdef unix
#include <stdlib.h>
#endif

namespace v360 {

class Variant{
public:
    enum TYPE { INT, BOOL, DOUBLE, STRING, UNSET };
    union DATA
    {
        int int_data;
        double double_data;
        bool bool_data;
        void* raw_data = NULL;
    };

    Variant(){
        type = UNSET;
    }
    Variant(bool value){
        data.bool_data = value;
        type = BOOL;
    }
    Variant(int value){
        data.int_data = value;
        type = INT;
    }
    Variant(double value){
        data.double_data = value;
        type = DOUBLE;
    }
    Variant(float value){
        data.double_data = value;
        type = DOUBLE;
    }
    Variant(std::string value){
        data.raw_data = new std::string;
        *(std::string*)data.raw_data = value;
        type = STRING;
    }
	Variant(const char* value, size_t length = -1){
		data.raw_data = new std::string(value, length != -1 ? length : strnlen(value, 100000000));
		type = STRING;
	}
    ~Variant(){
        deleteRaw();
    }
    Variant(const Variant& other ){
        *this = other;
    }
    Variant& operator=( const Variant& other ) {

        type = other.getType();
        if(type != STRING){
            data = other.getData();
        } else {
            deleteRaw();
            data.raw_data = new std::string;
            *(std::string*)data.raw_data = *(std::string*)other.getData().raw_data;
        }

        return *this;
    }
    inline void deleteRaw() {
        if(data.raw_data == NULL) return;
        if(type == STRING){
            delete (std::string*)data.raw_data;
            return;
        }
    }

    inline TYPE getType() const { return type; }
    bool isSet() const { return type != UNSET; }
    bool isBool() const { return type == BOOL; }
    bool isInt() const { return type == INT; }
    bool isDouble() const { return type == DOUBLE; }
    bool isString() const { return type == STRING; }
    bool toBool() const { return data.bool_data; }
    int toInt() const { return data.int_data; }
    double toDouble() const { return data.double_data; }
    const std::string& toString() const { return *(std::string*)data.raw_data; }

    std::ostream& outputToStream(std::ostream& ostr) const {
        
		switch (type){
		case DOUBLE:
			return ostr << data.double_data;
		case BOOL:
			if (data.bool_data) return ostr << "true";
			else				return ostr << "false";
		case INT:
			return ostr << data.int_data;
		case STRING:
			return ostr << *(std::string*)data.raw_data;
		case UNSET:
			return ostr << "(unset)";
		}
		return ostr << "(unknown)";
    }

private:
    DATA data;
    inline DATA getData() const { return data; }
    TYPE type;
};
inline std::ostream& operator<<(std::ostream& lhs, Variant const& value){
    return value.outputToStream(lhs);
}


class Remote {
private:

    PyObject *pName, *pModule, *pDict, *pFunc, *pClass, *pInstance;

public:

    Remote(){
        #ifdef unix
            setenv("PYTHONPATH",".",1);
        #endif

        Py_Initialize();

        PyRun_SimpleString("import os, sys"); 
        PyRun_SimpleString("sys.path.insert(0, os.path.abspath(\"..\"))");
        PyRun_SimpleString("import v360"); 

        pName = PyUnicode_FromString("v360");
        pModule = PyImport_Import(pName);

        if (pModule == NULL) {
            PyErr_Print();
            fprintf(stderr, "Failed to load \"v360\"\n");
            throw std::exception(); // TODO
        }

        pDict = PyModule_GetDict(pModule); // borrowed
        pClass = PyDict_GetItemString(pDict, "Remote"); // borrowed
        pInstance = PyObject_CallObject(pClass, NULL);
    }
    ~Remote(){
        
        Py_DECREF(pModule);
        Py_DECREF(pName);
        Py_DECREF(pClass);
        Py_DECREF(pDict);
		Py_XDECREF(pInstance);

        Py_Finalize();
    }

    inline Variant convertResult(PyObject* pValue){
        Variant result;
        if (pValue != NULL){
            if(PyFloat_Check(pValue)) result = Variant(PyFloat_AsDouble(pValue));
            if(PyBool_Check(pValue)) result = Variant((bool)(PyLong_AsLong(pValue) != 0));
            if(PyLong_CheckExact(pValue)) result = Variant((int)PyLong_AsLong(pValue));
			if(PyUnicode_Check(pValue)) {
				PyObject* ascii = PyUnicode_AsASCIIString(pValue);
				result = Variant(PyBytes_AsString(ascii), PyBytes_Size(ascii));
				Py_DECREF(ascii);
			}
            Py_DECREF(pValue);
        }else{
            PyErr_Print();
        } 
        return result;
    }

	inline bool returnBool(PyObject* pValue){
		Variant result = convertResult(pValue);
		if (!result.isBool()) return false;
		return result.toBool();
	}
	
#ifdef unix
	inline bool turnOn(const std::string& bluetooth_mac, const std::string& interface = "", int repeat = 3){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"turnOn", (char*)"(ssi)", bluetooth_mac.c_str(), interface.c_str(), repeat));
	}
#endif

	inline void turnOff(){
		Py_XDECREF(PyObject_CallMethod(pInstance, (char*)"turnOff", NULL));
	}
	
	inline bool connect(const std::string& pin = "0000", const std::string& token = ""){
		if (token == "") return returnBool(PyObject_CallMethod(pInstance, (char*)"connect", (char*)"(s)", pin.c_str()));
		return returnBool(PyObject_CallMethod(pInstance, (char*)"connect", (char*)"(ss)", pin.c_str(), token.c_str()));
	}

	inline void disconnect(){
		Py_XDECREF(PyObject_CallMethod(pInstance, (char*)"disconnect", NULL));
	}

	inline void setVerbose(bool verbose){
		PyObject_SetAttrString(pClass, (char*)"verbose", verbose ? Py_True : Py_False);
	}

	inline bool setVideoResolution(const std::string& resolution){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"setVideoResolution", (char*)"(s)", resolution.c_str()));
	}

	inline bool setVideoWhitebalance(const std::string& whitebalance){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"setVideoWhitebalance", (char*)"(s)", whitebalance.c_str()));
	}

	inline bool setVideoEffect(const std::string& effect){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"setVideoEffect", (char*)"(s)", effect.c_str()));
	}

	inline bool setMode(const std::string& mode){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"setMode", (char*)"(s)", mode.c_str()));
	}

	inline bool setAutoRotation(bool on){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"setAutoRotation", (char*)"(b)", on)); // TODO: Check (b), otherwise (i)
	}

	inline bool startVideo(){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"startVideo", NULL));
	}

	inline bool stopVideo(){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"stopVideo", NULL));
	}

	inline Variant getName(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getName", NULL));
	}

	inline Variant getBattery(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getBattery", NULL));
	}

	inline Variant getHdmiConnected(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getHdmiConnected", NULL));
	}

	inline Variant getPowerConnected(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPowerConnected", NULL));
	}

	inline Variant getVideoResolution(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getVideoResolution", NULL));
	}

	inline Variant getVideoEffect(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getVideoEffect", NULL));
	}

	inline Variant getVideoWhitebalance(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getVideoWhitebalance", NULL));
	}

	inline Variant getVideoTimelapseInterval(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getVideoTimelapseInterval", NULL));
	}

	inline Variant getPhotoBurstRate(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPhotoBurstRate", NULL));
	}

	inline Variant getPhotoEffect(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPhotoEffect", NULL));
	}

	inline Variant getPhotoExposure(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPhotoExposure", NULL));
	}

	inline Variant getPhotoResolution(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPhotoResolution", NULL));
	}

	inline Variant getPhotoTimelapseInterval(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPhotoTimelapseInterval", NULL));
	}

	inline Variant getPhotoWhitebalance(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getPhotoWhitebalance", NULL));
	}
};

}