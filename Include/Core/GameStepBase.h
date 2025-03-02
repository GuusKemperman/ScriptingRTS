#pragma once
#include "Commands/CommandBuffer.h"

namespace RTS
{
	template<typename... T>
	class GameStepBase;

	template <>
	class GameStepBase<>
	{
	public:
		void AddCommand() = delete;

		template<typename... Args>
		void ForEachCommandBuffer(Args&&...) {};

		template<typename... Args>
		void ForEachCommandBuffer(Args&&...) const {};
	};

	template<typename T, typename... O>
	class GameStepBase<T, O...> : public GameStepBase<O...>
	{
		using Base = GameStepBase<O...>;

	public:
		using Base::AddCommand;

		void AddCommand(T&& command)
		{
			mBuffer.AddCommand(std::move(command));
		}

		template<typename... Args>
		void ForEachCommandBuffer(const auto& func, Args&&... args)
		{
			func(mBuffer, args...);
			Base::ForEachCommandBuffer(func, args...);
		}

		template<typename... Args>
		void ForEachCommandBuffer(const auto& func, Args&&... args) const
		{
			func(mBuffer, args...);
			Base::ForEachCommandBuffer(func, args...);
		}

		template<typename C>
		CommandBuffer<C>& GetBuffer()
		{
			if constexpr (std::is_same_v<C, T>)
			{
				return mBuffer;
			}
			else
			{
				return Base::template GetBuffer<C>();
			}
		}

		template<typename C>
		const CommandBuffer<C>& GetBuffer() const
		{
			if constexpr (std::is_same_v<C, T>)
			{
				return mBuffer;
			}
			else
			{
				return Base::template GetBuffer<C>();
			}
		}

	private:
		CommandBuffer<T> mBuffer{};
	};
}