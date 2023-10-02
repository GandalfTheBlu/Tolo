#include "src/program_handle.h"
#include "src/file_io.h"
#include <cmath>
#include <filesystem>
#include <set>
#include <iostream>

using namespace Tolo;

struct vec3
{
	float x, y, z;

	vec3() :
		x(0.f),
		y(0.f),
		z(0.f)
	{}

	vec3(float _x, float _y, float _z) :
		x(_x),
		y(_y),
		z(_z)
	{}

	vec3(const vec3& rhs) :
		x(rhs.x),
		y(rhs.y),
		z(rhs.z)
	{}

	vec3& operator=(const vec3& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}
};

float length(const vec3& v)
{
	return std::sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

vec3 normalize(const vec3& v)
{
	float s = 1.f / length(v);
	return vec3{ v.x * s, v.y * s, v.z * s };
}

vec3 add(const vec3& a, const vec3& b)
{
	return vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}

vec3 sub(const vec3& a, const vec3& b)
{
	return vec3{ a.x - b.x, a.y - b.y, a.z - b.z };
}

vec3 mul(const vec3& a, float s)
{
	return vec3{ a.x * s, a.y * s, a.z * s };
}

float dot(const vec3& a, const vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}


enum class MessageType
{
	Empty,
	FileUpdated
};

struct Message
{
	MessageType type;

	Message() :
		type(MessageType::Empty)
	{}

	Message(MessageType _type) :
		type(_type)
	{}
};

class MessageSubscriber
{
public:
	virtual void ReceiveMessage(const Message& message) = 0;
};

class MessageChannel
{
private:
	std::map<MessageType, std::vector<MessageSubscriber*>> msgTypeToSubscribers;

	MessageChannel(const MessageChannel&) = delete;
	MessageChannel& operator=(const MessageChannel&) = delete;

public:
	MessageChannel()
	{}

	void Subscribe(MessageSubscriber* p_subscriber, MessageType type)
	{
		std::vector<MessageSubscriber*>& subscribers = msgTypeToSubscribers[type];
		if (std::find(subscribers.begin(), subscribers.end(), p_subscriber) == subscribers.end())
			subscribers.push_back(p_subscriber);
	}

	void Unsubscribe(MessageSubscriber* p_subscriber, MessageType type)
	{
		if (msgTypeToSubscribers.count(type) == 0)
			return;

		std::vector<MessageSubscriber*>& subscribers = msgTypeToSubscribers[type];
		auto subItr = std::find(subscribers.begin(), subscribers.end(), p_subscriber);
		if (subItr != subscribers.end())
			subscribers.erase(subItr);
	}

	void PublishMessage(const Message& message)
	{
		if (msgTypeToSubscribers.count(message.type) == 0)
			return;

		std::vector<MessageSubscriber*>& subscribers = msgTypeToSubscribers[message.type];

		for (int i = (int)subscribers.size() - 1; i >= 0; i--)
			subscribers[i]->ReceiveMessage(message);
	}
};

class FileListener
{
private:
	std::string filePath;
	MessageChannel msgChannel;
	std::chrono::seconds::rep currentFileChangedTime;

	std::chrono::seconds::rep GetFileChangedTime()
	{
		std::filesystem::path p = filePath;
		std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(p);
		return std::chrono::duration_cast<std::chrono::seconds>(writeTime.time_since_epoch()).count();
	}

	FileListener() = delete;
	FileListener(const FileListener&) = delete;
	FileListener& operator=(const FileListener&) = delete;

public:
	FileListener(const std::string& _filePath) :
		filePath(_filePath)
	{
		currentFileChangedTime = GetFileChangedTime();
	}

	void Subscribe(MessageSubscriber* p_subscriber)
	{
		msgChannel.Subscribe(p_subscriber, MessageType::FileUpdated);
	}

	void Unsubscribe(MessageSubscriber* p_subscriber)
	{
		msgChannel.Unsubscribe(p_subscriber, MessageType::FileUpdated);
	}

	void Update()
	{
		auto updatedTime = GetFileChangedTime();
		if (updatedTime > currentFileChangedTime)
		{
			currentFileChangedTime = updatedTime;
			msgChannel.PublishMessage(Message(MessageType::FileUpdated));
		}
	}
};

class FileManager
{
private:
	std::map<std::string, FileListener*> listeners;

	FileManager()
	{}

public:
	~FileManager()
	{
		for (auto e : listeners)
			delete e.second;
	}

	static FileManager& Instance()
	{
		static FileManager instance;
		return instance;
	}

	void SubscribeToFileChange(MessageSubscriber* p_subscriber, const std::string& filePath)
	{
		if (listeners.count(filePath) == 0)
			listeners[filePath] = new FileListener(filePath);

		listeners[filePath]->Subscribe(p_subscriber);
	}

	void UnsubscribeToFileChange(MessageSubscriber* p_subscriber, const std::string& filePath)
	{
		if (listeners.count(filePath) == 0)
			return;

		listeners[filePath]->Unsubscribe(p_subscriber);
	}

	void Update()
	{
		for (auto& e : listeners)
			e.second->Update();
	}
};

class SDF_Object : public MessageSubscriber
{
private:
	ProgramHandle toloProgram;

	SDF_Object() = delete;
	SDF_Object(const SDF_Object&) = delete;
	SDF_Object& operator=(const SDF_Object&) = delete;

public:
	SDF_Object(const std::string& _toloCodePath, Ptr stackSize) :
		toloProgram(_toloCodePath, stackSize, "sdf")
	{
		FileManager::Instance().SubscribeToFileChange(this, _toloCodePath);

		toloProgram.AddStruct({
			"vec3",
			{
				{"float", "x"},
				{"float", "y"},
				{"float", "z"}
			}
		});
		toloProgram.AddFunction({ "vec3", "operator+", {"vec3", "vec3"}, [](VirtualMachine& vm)
			{
				vec3 a = Pop<vec3>(vm);
				vec3 b = Pop<vec3>(vm);
				PushStruct<vec3>(vm, add(a, b));
			}
		});
		toloProgram.AddFunction({ "vec3", "operator-", {"vec3", "vec3"}, [](VirtualMachine& vm)
			{
				vec3 a = Pop<vec3>(vm);
				vec3 b = Pop<vec3>(vm);
				PushStruct<vec3>(vm, sub(a, b));
			}
		});
		toloProgram.AddFunction({ "vec3", "operator-", {"vec3"}, [](VirtualMachine& vm)
			{
				vec3 a = Pop<vec3>(vm);
				PushStruct<vec3>(vm, vec3{-a.x, -a.y, -a.z});
			}
		});
		toloProgram.AddFunction({ "vec3", "operator*", {"vec3", "float"}, [](VirtualMachine& vm)
			{
				vec3 a = Pop<vec3>(vm);
				float b = Pop<float>(vm);
				PushStruct<vec3>(vm, mul(a, b));
			}
		});
		toloProgram.AddFunction({ "float", "length", {"vec3"}, [](VirtualMachine& vm)
			{
				vec3 v = Pop<vec3>(vm);
				Push<Float>(vm, length(v));
			}
		});
	}

	~SDF_Object()
	{
		FileManager::Instance().UnsubscribeToFileChange(this, toloProgram.GetCodePath());
	}

	virtual void ReceiveMessage(const Message& message) override
	{
		switch (message.type)
		{
		case MessageType::Empty:
			return;
		case MessageType::FileUpdated:
			std::cout << "detected update in '" << toloProgram.GetCodePath() << "', will try to hot reload" << std::endl;
			Compile();
			break;
		}
	}

	void Compile()
	{
		std::string outToloCode;
		toloProgram.Compile(outToloCode);

		// future: use outToloCode to append to fragment shader code, then recompile the shader
		std::cout << "future: recompiling sdf shader" << std::endl;
	}

	// future: use this for collision detection
	float GetDistance(const vec3& point)
	{
		return toloProgram.Execute<float>(point);
	}
};


float march(vec3 origin, vec3 dir, SDF_Object& obj)
{
	float t = 0.0;
	int i = 0;
	while (i < 32)
	{
		float r = obj.GetDistance(add(origin, mul(dir, t)));

		if (r < 0.001)
		{
			break;
		}

		t = t + r;
		i = i + 1;
	}

	return t;
}

void Render(SDF_Object& obj)
{
	std::string grid;
	size_t width = 40;
	size_t height = 40;
	grid.resize((2 * width + 1) * height, ' ');

	vec3 origin{ 0.f, 0.f, -3.f };
	vec3 toLight = normalize(vec3{ 1.f, 1.f, -1.f });

	char gradient[3] = { '.', '*', '@' };

	size_t index = 0;
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			float u = (float)x / (width - 1);
			float v = 1.f - (float)y / (height - 1);
			float vx = 2.f * u - 1.f;
			float vy = 2.f * v - 1.f;
			vec3 dir = normalize(vec3{ vx, vy, 2.f });

			float t = march(origin, dir, obj);

			if (t > 20.f)
				grid[index] = ' ';
			else
			{
				vec3 point = add(origin, mul(dir, t));
				vec3 norm = normalize(point);
				float dif = std::max(dot(norm, toLight), 0.f);

				grid[index] = gradient[std::min(int(dif * 3.f), 2)];
			}

			index += 2;
		}
		grid[index++] = '\n';
	}

	std::printf("%s", grid.c_str());
}

int main()
{
	SDF_Object sdfObject("Script/sdf.tolo", 1024);
	sdfObject.Compile();

	while (true)
	{
		FileManager::Instance().Update();

		Render(sdfObject);

		std::cin.get();
	}

	return 0;
}