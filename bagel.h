// Copyright (C) 2026 Moshe Sulamy

#pragma once
#include <cstdlib>
#include <cstdint>
#include <algorithm>

namespace bagel
{
	using id_type = int;
	using ent_type = struct { id_type id; };

	struct NoInstance {	NoInstance() = delete; };
	struct NoCopy {
		NoCopy() = default; // default constructor
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;
	};

	template <class T, int N>
	class DynamicBag : NoCopy
	{
	public:
		int size() const { return _size; }
		void ensure(int new_capacity) {
			if (new_capacity > _capacity) {
				_capacity = std::max(_capacity*2, new_capacity);
				_arr = static_cast<T*>(
					realloc(_arr, sizeof(T)*_capacity));
			}
		}
		void push(const T& val) {
			if (_size == _capacity) {
				_capacity *= 2;
				_arr = static_cast<T*>(
					realloc(_arr, sizeof(T)*_capacity));
			}
			_arr[_size] = val;
			++_size;
		}
		T pop() {
			return _arr[--_size];
		}
		~DynamicBag() {
			free(_arr);
		}

		T& operator[](int idx) { return _arr[idx]; }
		const T& operator[](int idx) const { return _arr[idx]; }
	private:
		T*		_arr = static_cast<T*>(malloc(sizeof(T) * N));
		int		_size = 0;
		int		_capacity = N;
	};

	template <class T>
	class SparseStorage final : NoInstance
	{
	public:
		static void add(ent_type ent, const T& val) {
			_comps.ensure(ent.id+1);
			_comps[ent.id] = val;
		}
		static void del(ent_type) {}
		static T& get(ent_type ent) {
			return _comps[ent.id];
		}
	private:
		static inline DynamicBag<T,100> _comps;
	};
	template <class T>
	class TaggedStorage final : NoInstance
	{
	public:
		static void add(ent_type, const T&) {}
		static void del(ent_type) {}
		static T& get(ent_type) = delete;
	};
	template <class T>
	class PackedStorage final : NoInstance
	{
	public:
		static void add(const ent_type ent, const T& val) {
			_idToComp.ensure(ent.id+1);
			_idToComp[ent.id] = _comps.size();
			_comps.push(val);
			_compToId.push(ent.id);
		}
		static void del(const ent_type ent) {
			int idx = _idToComp[ent.id];
			const id_type last = _compToId.pop();

			_comps[idx] = _comps.pop();
			_compToId[idx] = last;
			_idToComp[last] = idx;
		}
		static T& get(const ent_type ent) {
			return _comps[_idToComp[ent.id]];
		}
	private:
		static inline DynamicBag<T,100> _comps;
		static inline DynamicBag<int,100> _idToComp;
		static inline DynamicBag<id_type,100> _compToId;
	};
	template <class T>
	class StackStorage final : NoInstance
	{
	public:
		static void add(const ent_type ent, const T& val) {
			_idToComp.ensure(ent.id+1);
			if (_freeIdx.size() > 0) {
				const int idx = _freeIdx.pop();
				_idToComp[ent.id] = idx;
				_comps[idx] = val;
			}
			else {
				_idToComp.ensure(ent.id+1);
				_idToComp[ent.id] = _comps.size();
				_comps.push(val);
			}
			//TODO: remember empty/full cells
		}
		static void del(const ent_type ent) {
			_freeIdx.push(_idToComp[ent.id]);
		}
		static T& get(const ent_type ent) {
			return _comps[_idToComp[ent.id]];
		}
	private:
		static inline DynamicBag<T,100> _comps;
		static inline DynamicBag<int,100> _idToComp;
		static inline DynamicBag<id_type,100> _freeIdx;
	};

	template <class T>
	struct Storage final : NoInstance {
		using type = SparseStorage<T>;
	};

	class Mask final
	{
	public:
		using bit_type = std::uint_fast64_t;
		using mask_type = std::uint_fast64_t;
		static constexpr bit_type bit(const int idx) { return 1<<idx; }

		void set(const bit_type b) { _mask |= b; }

		void clear(const bit_type b) { _mask &= ~b; }
		void clear() { _mask = 0; }

		bool test(const bit_type b) const { return _mask & b; }
		bool test(const Mask m) const { return (_mask & m._mask) == m._mask; }
	private:
		mask_type	_mask{0};
	};

	static inline int compCounter = -1;
	template <class>
	struct Component final : NoInstance
	{
		static inline const int						Index = ++compCounter;
		static inline const Mask::bit_type	Bit = Mask::bit(Index);
	};

	class World final : NoInstance
	{
	public:
		static ent_type createEntity() {
			if (_ids.size() > 0)
				return {_ids.pop()};
			_masks.push(Mask{});
			return {++_maxId};
		}
		static void deleteEntity(ent_type ent) {
			_masks[ent.id].clear();
			_ids.push(ent.id);
			//TODO: delete components
		}

		static const Mask& mask(ent_type e) {
			return _masks[e.id];
		}
		template <class T>
		static T& getComponent(ent_type e) {
			return Storage<T>::type::get(e);
		}
		template <class T>
		static void addComponent(ent_type ent, const T& comp) {
			_masks[ent.id].set(Component<T>::Bit);
			Storage<T>::type::add(ent,comp);
		}
		template <class T>
		static void delComponent(ent_type ent, const T& comp) {
			_masks[ent.id].clear(Component<T>::Bit);
			Storage<T>::type::del(ent,comp);
		}

		static id_type maxId() { return _maxId; }
	private:
		static inline DynamicBag<Mask,100>	_masks;
		static inline DynamicBag<id_type,100>		_ids;
		static inline id_type _maxId = -1;
	};

	class MaskBuilder
	{
	public:
		template <class T>
		MaskBuilder& set() {
			m.set(Component<T>::Bit);
			return *this;
		}
		Mask build() const { return m; }
	private:
		Mask m;
	};
}