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
        //assert(type != UNSET);
        if(type == DOUBLE){
            return ostr << data.double_data;
        } else if(type == BOOL){
            if(data.bool_data)
                return ostr << "true";
            else
                return ostr << "false";
        } else if(type == INT){
            return ostr << data.int_data;
        }else if(type == STRING){
            return ostr << *(std::string*)data.raw_data;
        }
        return ostr;
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

    inline Variant getName(){
		return convertResult(PyObject_CallMethod(pInstance, (char*)"getName", NULL));
    }
	
	inline bool turnOn(const std::string& bluetooth_mac, const std::string& interface = "", int repeat = 3){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"turnOn", (char*)"(ssi)", bluetooth_mac.c_str(), interface.c_str(), repeat));
	}

	inline void turnOff(){
		Py_XDECREF(PyObject_CallMethod(pInstance, (char*)"turnOff", NULL));
	}
	
	inline bool connect(const std::string& pin = "0000", const std::string& token = ""){
		return returnBool(PyObject_CallMethod(pInstance, (char*)"connect", (char*)"(ss)", pin.c_str(), token.c_str()));
	}

	inline void disconnect(){
		Py_XDECREF(PyObject_CallMethod(pInstance, (char*)"disconnect", NULL));
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

	/* TODO:
	
		def getName(self):
		return self.returnCached(self.camera_info, ("caminf", "name"), False)

	def getBattery(self):
		return self.returnCached(self.camera_status, "batlev")

	def getHdmiConnected(self):
		return self.returnCached(self.camera_status, "hdmist")

	def getPowerConnected(self):
		return self.returnCached(self.camera_status, "powsrc")

	def getVideoResolution(self):
		return self.returnCached(self.video_settings, "vdores")

	def getVideoEffect(self):
		return self.returnCached(self.video_settings, "vdocef")

	def getVideoWhitebalance(self):
		return self.returnCached(self.video_settings, "vdowbl")

	def getVideoTimelapseInterval(self):
		return self.returnCached(self.video_settings, ("vdotlp", "interval"))

	def getVideoResolution(self):
		return self.returnCached(self.video_settings, "vdores")

	def getPhotoBurstRate(self):
		return self.returnCached(self.photo_settings, ("picbur", "rate"))

	def getPhotoEffect(self):
		return self.returnCached(self.photo_settings, "piccef")

	def getPhotoExposure(self):
		return self.returnCached(self.photo_settings, "picexp")		

	def getPhotoResolution(self):
		return self.returnCached(self.photo_settings, "picres")

	def getPhotoTimelapseInterval(self):
		return self.returnCached(self.photo_settings, ("pictlp", "interval"))				

	def getPhotoWhitebalance(self):
		return self.returnCached(self.photo_settings, "picwbl")	

	def getPhotoPictwbInterval(self): # TODO: unknown setting, rename 
		return self.returnCached(self.photo_settings, ("pictwb", "interval"))

	def getPhotoPictwbRate(self): # TODO: unknown setting, rename
		return self.returnCached(self.photo_settings, ("pictwb", "rate"))	

	*/

};

}