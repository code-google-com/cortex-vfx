The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#summary useful snippets!

# IECore.MeshPrimitive: find connected faces in a merged mesh #

I had this problem were I had to assign per-vertex primitive variable to different meshes that have being merged into one big mesh.

So I've wrote this snippet to identify all the meshes in a merged mesh by splitting the  faces of the mesh in groups were the faces share the same vertices.

```
import maya.cmds as m
import IECore
import IECoreMaya

def getFaces( meshName ):
    cortexMesh = IECoreMaya.FromMayaMeshConverter( meshName ).convert()
    vcount = 0
    faces=[]
    for f in range(len(cortexMesh.verticesPerFace)):
        faces.append([])
        for each in range(cortexMesh.verticesPerFace[f]):
            faces[f].append(cortexMesh.vertexIds[vcount])
            vcount += 1
    return faces

def mesh(faceID, faces, r=[]):
    r.append(faceID)
    for v in faces[faceID]:
        for fid in range(len(faces)):
            if v in faces[fid]:
                if fid not in r:
                    r = mesh( fid, faces, r)
    return r

def getMeshes(faces):
    res=[]
    for faceID in range(len(faces)):
        m = mesh(faceID, faces, [])
        found = False
        for fid in m:
            for r in res:
                if fid in r:
                     found = True
                     break
        if not found:
            res.append(m)
    return res
        

all = m.ls(sl=True, dag=True, ni=True, type='mesh')
baseUVs = {}
for each in all:
    faces = getFaces( each )
    meshes = getMeshes(faces)
    print "merged mesh:", each
    for submesh in range(len(meshes)):
        print "sub-mesh %d:" % submesh, "faces: %s" % str(meshes[submesh])
        
```