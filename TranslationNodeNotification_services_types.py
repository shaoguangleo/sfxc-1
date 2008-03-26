################################################## 
# TranslationNodeNotification_services_types.py 
# generated by ZSI.generate.wsdl2python
##################################################


import ZSI
import ZSI.TCcompound
from ZSI.schema import LocalElementDeclaration, ElementDeclaration, TypeDefinition, GTD, GED
from ZSI.generate.pyclass import pyclass_type

##############################
# targetNamespace
# http://jobinfo.broker.remote.expres.psnc.pl/xsd
##############################

class ns1:
    targetNamespace = "http://jobinfo.broker.remote.expres.psnc.pl/xsd"

    class ChunkInfo_Def(ZSI.TCcompound.ComplexType, TypeDefinition):
        schema = "http://jobinfo.broker.remote.expres.psnc.pl/xsd"
        type = (schema, "ChunkInfo")
        def __init__(self, pname, ofwhat=(), attributes=None, extend=False, restrict=False, **kw):
            ns = ns1.ChunkInfo_Def.schema
            TClist = [ZSI.TCnumbers.Ilong(pname=(ns,"chunkId"), aname="_chunkId", minOccurs=0, maxOccurs=1, nillable=False, typed=False, encoded=kw.get("encoded")), ZSI.TC.String(pname=(ns,"chunkLocation"), aname="_chunkLocation", minOccurs=0, maxOccurs=1, nillable=True, typed=False, encoded=kw.get("encoded")), ZSI.TCnumbers.Ilong(pname=(ns,"chunkSize"), aname="_chunkSize", minOccurs=0, maxOccurs=1, nillable=False, typed=False, encoded=kw.get("encoded")), ZSI.TC.String(pname=(ns,"endTime"), aname="_endTime", minOccurs=0, maxOccurs=1, nillable=True, typed=False, encoded=kw.get("encoded")), ZSI.TC.String(pname=(ns,"startTime"), aname="_startTime", minOccurs=0, maxOccurs=1, nillable=True, typed=False, encoded=kw.get("encoded")), ZSI.TC.String(pname=(ns,"translationNodeIP"), aname="_translationNodeIP", minOccurs=0, maxOccurs=1, nillable=True, typed=False, encoded=kw.get("encoded")), ZSI.TCnumbers.Iint(pname=(ns,"translationNodeId"), aname="_translationNodeId", minOccurs=0, maxOccurs=1, nillable=False, typed=False, encoded=kw.get("encoded"))]
            self.attribute_typecode_dict = attributes or {}
            if extend: TClist += ofwhat
            if restrict: TClist = ofwhat
            ZSI.TCcompound.ComplexType.__init__(self, None, TClist, pname=pname, inorder=0, **kw)
            class Holder:
                __metaclass__ = pyclass_type
                typecode = self
                def __init__(self):
                    # pyclass
                    self._chunkId = None
                    self._chunkLocation = None
                    self._chunkSize = None
                    self._endTime = None
                    self._startTime = None
                    self._translationNodeIP = None
                    self._translationNodeId = None
                    return
            Holder.__name__ = "ChunkInfo_Holder"
            self.pyclass = Holder

# end class ns1 (tns: http://jobinfo.broker.remote.expres.psnc.pl/xsd)

##############################
# targetNamespace
# http://broker.remote.expres.psnc.pl/xsd
##############################

class ns0:
    targetNamespace = "http://broker.remote.expres.psnc.pl/xsd"

    class chunkIsReady_Dec(ZSI.TCcompound.ComplexType, ElementDeclaration):
        literal = "chunkIsReady"
        schema = "http://broker.remote.expres.psnc.pl/xsd"
        def __init__(self, **kw):
            ns = ns0.chunkIsReady_Dec.schema
            TClist = [GTD("http://jobinfo.broker.remote.expres.psnc.pl/xsd","ChunkInfo",lazy=False)(pname=(ns,"param0"), aname="_param0", minOccurs=0, maxOccurs=1, nillable=True, typed=False, encoded=kw.get("encoded"))]
            kw["pname"] = ("http://broker.remote.expres.psnc.pl/xsd","chunkIsReady")
            kw["aname"] = "_chunkIsReady"
            self.attribute_typecode_dict = {}
            ZSI.TCcompound.ComplexType.__init__(self,None,TClist,inorder=0,**kw)
            class Holder:
                __metaclass__ = pyclass_type
                typecode = self
                def __init__(self):
                    # pyclass
                    self._param0 = None
                    return
            Holder.__name__ = "chunkIsReady_Holder"
            self.pyclass = Holder

# end class ns0 (tns: http://broker.remote.expres.psnc.pl/xsd)