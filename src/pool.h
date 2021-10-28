#pragma once

template< typename T >
class Pool
{
public:
	using Create = T*();
	using Destroy = void( T** );
	using Reset = void( T* );

	Pool( Create*, Destroy*, Reset* );
	~Pool();

	T* Aquire();
	void Release( T*& );

	template< Container< T* > C >
	void Release( C&& );

private:
	Create* create;
	Destroy* destroy;
	Reset* reset;

	std::deque< T* > items;
};

template< typename T >
Pool<T>::Pool( Create* crt, Destroy* dstr, Reset* rst ) : create(crt), destroy(dstr), reset(rst)
{
}

template< typename T >
Pool<T>::~Pool()
{
	for( auto* item : items )
	{
		destroy( &item );
	}
}

template< typename T >
T* Pool<T>::Aquire()
{
	if( !items.empty() )
	{
		auto* item = items.front();
		items.pop_front();

		return item;
	}

	return create();
}

template< typename T >
void Pool<T>::Release( T*& item )
{
	reset(item);
	items.push_back(item);

	item = nullptr;
}

template< typename T >
template< Container< T* > C >
void Pool<T>::Release( C&& cont )
{
	for( auto* item : cont )
	{
		Release(item);
	}

	if constexpr( !std::is_const_v< std::remove_reference_t<C> > )
	{
		cont.clear();
	}
}
