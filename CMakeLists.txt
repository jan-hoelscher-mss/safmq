cmake_minimum_required(VERSION 2.8)
project (safmq)

set(CMAKE_VERBOSE_MAKEFILE Off)
SET(BUILD_SHARED_LIBS Off)
include_directories (.)
include_directories (./shared)

#add_library (safmq
SET(SAFMQ_SOURCE
    ${SAFMQ_SOURCE}
	Signal.cpp 
	SystemDelivery.cpp 
	Log.cpp 
	logger.cpp 
	DirectoryList.cpp 
	Mutex.cpp 
	QStorage.cpp 
	ServiceThread.cpp 
	QManager.cpp 
	ServerThread.cpp 
	shared/thdlib.cpp 
	QAccessControl.cpp 
	param_config.cpp 
	ForwardThread.cpp 
	TransactionManager.cpp 
	SystemConnection.cpp 
	Command.cpp 
	AddressParser.cpp 
	Stats.cpp
	shared/tcpsocket/tcpsocket.cpp
	lib/MQConnection.cpp
	lib/MQFactory.cpp
	lib/MessageQueue.cpp
)
IF(NOT WIN32)
  SET(SAFMQ_SOURCE
    ${SAFMQ_SOURCE}
	SysLogger.cpp
	shared/uuidgen/uuidgen.c
  )
ELSEIF(APPLE)

ELSEIF(WIN32)
  SET(SAFMQ_SOURCE
    ${SAFMQ_SOURCE}
	NTSysLogger.cpp 
	shared/uuidgen/uuidgenwin.c
	ntservice.cpp 


)

ENDIF(NOT WIN32)
add_library (safmq ${SAFMQ_SOURCE})

IF(UNIX AND NOT APPLE)
	ADD_DEFINITIONS(-DCONFIG_DIR="")
	ADD_DEFINITIONS(-DCONFIG_ENV="\\"SACFG\\"")
	ADD_DEFINITIONS(-DUUID_USE_INTTYPES)
	ADD_DEFINITIONS(-DSAFMQ_POSIX_HWADDR)
ELSEIF(APPLE)
	ADD_DEFINITIONS(-DCONFIG_DIR='\\"\\"')
	ADD_DEFINITIONS(-DCONFIG_ENV="\\"SACFG\\"")
	ADD_DEFINITIONS(-DUUID_USE_INTTYPES)
ELSEIF(WIN32)
	SET(EXTRA_LIBS ws2_32 rpcrt4)
	ADD_DEFINITIONS(-DCONFIG_DIR='\\"\\"')
	ADD_DEFINITIONS(-DCONFIG_ENV="\\"SACFG\\"")
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS )
	ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS )
ENDIF(UNIX AND NOT APPLE)

target_link_libraries (safmq ${EXTRA_LIBS})

INSTALL(TARGETS safmq
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
    ARCHIVE DESTINATION lib
)
FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${files} DESTINATION include) 

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h")
INSTALL(FILES ${files} DESTINATION include/lib) 

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/shared/tcpsocket/*.h")
INSTALL(FILES ${files} DESTINATION include/shared/tcpsocket)
            



