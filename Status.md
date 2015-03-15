The cortex wiki is now hosted at https://github.com/ImageEngine/cortex/wiki
#labels Featured
  * The IECore, IECoreRI, IECoreGL, and IECoreMaya libraries are now in the repository. All internal Image Engine development will be committed to the trunk.

  * Image Engine are now using the provided SCons-based build setup for all their internal builds

  * IECore has been successfully built and tested on the follow platforms:
    * Redhat Enterprise Linux 4 / ia32
      * gcc 3.3.2 / gcc 3.3.4 / gcc 3.4.6 / gcc 4.0.1 / gcc 4.0.2 / gcc 4.1.2
    * Ubuntu 8.01 / x86\_64 :
      * gcc 3.4.6 / gcc 4.1.2 / gcc 4.3.0
    * CentOS 5 / x86\_64 :
      * gcc 3.4.6 / gcc 4.0.4 / gcc 4.1.2
    * Mac OSX 10.4 / PowerPC
      * gcc 4.0.1
    * Mac OSX 10.4 / ia32
      * gcc 4.0.1
    * Mac OSX 10.5.8 / ia32 (Leopard)
      * gcc 4.0.1
    * Mac OSX 10.6.1 / x64 (Snow Leopard)
      * gcc 4.2.1
      * Note that it is necessary to set MACOSX\_DEPLOYMENT\_TARGET=10.4 to workaround problems introduced by the Snow leopard dynamic linker.

> NB. gcc 4.2.x exhibits buggy behaviour