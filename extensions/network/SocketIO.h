/****************************************************************************
 Copyright (c) 2010-2013 cocos2d-x.org
 Copyright (c) 2013 Chris Hannon http://www.channon.us
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*based on the SocketIO library created by LearnBoost at http://socket.io
*using spec version 1 found at https://github.com/LearnBoost/socket.io-spec

Usage is described below, a full working example can be found in TestCpp under ExtionsTest/NetworkTest/SocketIOTest

creating a new connection to a socket.io server running at localhost:3000

	SIOClient *client = SocketIO::connect(*delegate, "ws://localhost:3000");

the connection process will begin and if successful delegate::onOpen will be called
if the connection process results in an error, delegate::onError will be called with the err msg

sending a message to the server

	client->send("Hello!");

emitting an event to be handled by the server, argument json formatting is up to you

	client->emit("eventname", "[{\"arg\":\"value\"}]");

registering an event callback, target should be a member function in a subclass of SIODelegate
CC_CALLBACK_2 is used to wrap the callback with std::bind and store as an SIOEvent

	client->on("eventname", CC_CALLBACK_2(TargetClass::targetfunc, *targetclass_instance));

event target function should match this pattern, *this pointer will be made available

	void TargetClass::targetfunc(SIOClient *, const std::string&)

disconnect from the endpoint by calling disconnect(), onClose will be called on the delegate once complete
in the onClose method the pointer should be set to NULL or used to connect to a new endpoint

	client->disconnect();

 ****************************************************************************/

#ifndef __CC_SOCKETIO_H__
#define __CC_SOCKETIO_H__

#include "ExtensionMacros.h"
#include "cocos2d.h"

NS_CC_EXT_BEGIN

//forward declarations
class SIOClientImpl;
class SIOClient;

/**
 *  @brief Singleton and wrapper class to provide static creation method as well as registry of all sockets
 */
class SocketIO
{
public:
	SocketIO();
	virtual ~SocketIO(void);

	static SocketIO *instance();

	/**
     *  @brief The delegate class to process socket.io events
     */
	class SIODelegate
    {
    public:
        virtual ~SIODelegate() {}
		virtual void onConnect(SIOClient* client) = 0;
        virtual void onMessage(SIOClient* client, const std::string& data) = 0;
        virtual void onClose(SIOClient* client) = 0;
        virtual void onError(SIOClient* client, const std::string& data) = 0;
    };

	/**
     *  @brief  Static client creation method, similar to socketio.connect(uri) in JS
     *  @param  delegate The delegate which want to receive events from the socket.io client
     *  @param  uri      The URI of the socket.io server
     *  @return An initialized SIOClient if connected successfully, otherwise NULL
     */
	static SIOClient* connect(SocketIO::SIODelegate& delegate, const std::string& uri);
	
private:

	static SocketIO *_inst;

	Dictionary* _sockets;

	SIOClientImpl* getSocket(const std::string& uri);
	void addSocket(const std::string& uri, SIOClientImpl* socket);
	void removeSocket(const std::string& uri);

	friend class SIOClientImpl;
	
};

//c++11 style callbacks entities will be created using CC_CALLBACK (which uses std::bind)
typedef std::function<void(SIOClient*, const std::string&)> SIOEvent;
//c++11 map to callbacks
typedef std::map<std::string, SIOEvent> EventRegistry;

/**
     *  @brief A single connection to a socket.io endpoint
     */
class SIOClient
	: public Object
{
private:
	int _port;
	std::string _host, _path, _tag;
	bool _connected;
	SIOClientImpl* _socket;

	SocketIO::SIODelegate* _delegate;

	EventRegistry _eventRegistry;

	void fireEvent(const std::string& eventName, const std::string& data);

	void onOpen();
	void onConnect();
	void receivedDisconnect();

	friend class SIOClientImpl;

public:
	SIOClient(const std::string& host, int port, const std::string& path, SIOClientImpl* impl, SocketIO::SIODelegate& delegate);
	virtual ~SIOClient(void);

	/**
     *  @brief Returns the delegate for the client
     */
	SocketIO::SIODelegate* getDelegate() { return _delegate; };

	/**
     *  @brief Disconnect from the endpoint, onClose will be called on the delegate when comlpete
     */
	void disconnect();
	/**
     *  @brief Send a message to the socket.io server
     */
	void send(std::string s);
	/**
     *  @brief The delegate class to process socket.io events
     */
	void emit(std::string eventname, std::string args);
	/**
     *  @brief Used to resgister a socket.io event callback
	 *		   Event argument should be passed using CC_CALLBACK2(&Base::function, this)
     */
	void on(const std::string& eventName, SIOEvent e);
	
	inline void setTag(const char* tag)
    {
        _tag = tag;
    };
    
	inline const char* getTag()
    {
        return _tag.c_str();
    };

};

NS_CC_EXT_END

#endif /* defined(__CC_JSB_SOCKETIO_H__) */
