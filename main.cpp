#include <iostream>
#include <bitset>
#include <map>
#include <variant>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include <absl/random/random.h>
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>

namespace {
absl::BitGen g_bitgen;
}

template<typename T,size_t N>
class IndexedDict
{
private:
	uint32_t _lookup[(N+31)/32]{};
	std::vector<T> _data;
public:
	using const_iterator = typename std::vector<T>::const_iterator;

	size_t size() const { return _data.size(); }

	void emplace( int k, const T &v )
	{
		// find the word
		// find the bit
		// if set return

	}

	const_iterator end() const
	{
		return _data.end();
	}

	const_iterator find( int k ) const
	{
		return _data.end();
	}
};

auto randomString( int i_min, int i_max )
{
	auto n = absl::Uniform<int>( g_bitgen, i_min, i_max );
	std::string s;
	s.reserve( n );
	for ( int i = 0; i < n; ++i )
		s.push_back( absl::Uniform<int>( g_bitgen, 'a', 'z' ) );
	return s;
}

auto generateAllAttribs( int i_min, int i_max, int i_nb )
{
	std::set<std::string> result;

	while ( result.size() < i_nb )
		result.emplace( randomString( i_min, i_max ) );

	return std::vector<std::string>( result.begin(), result.end() );
}

auto print( std::chrono::high_resolution_clock::duration d )
{
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>( d ).count();
	if ( ms >= 10 )
		return std::to_string( ms ) + "ms";
	auto us = std::chrono::duration_cast<std::chrono::microseconds>( d ).count();
	return std::to_string( us ) + "us";
}

void randomizeHeap( std::vector<std::unique_ptr<char[]>> &randomAllocs )
{
	auto previousSize = randomAllocs.size();
	// allocate new
	auto newAlloc = absl::Uniform<int>( g_bitgen, 2, 10 );
	for ( int i = 0; i < newAlloc; ++i )
	{
		randomAllocs.emplace_back(
			std::make_unique<char[]>(
				absl::Uniform<size_t>( g_bitgen, 24, 65536 )
			));
	}
	if ( previousSize == 0 )
		return;

	auto toDeallocate = std::min( previousSize, absl::Uniform<size_t>( g_bitgen, 1, 5 ) );
	auto index = absl::Uniform<size_t>( g_bitgen, 0, previousSize - 1 );
	while ( toDeallocate > 0 )
	{
		randomAllocs.erase( randomAllocs.begin() + index );
		index += 2;
		if ( index >= randomAllocs.size() )
			index = 0;
		--toDeallocate;
	}
}

template<typename T, typename RANDOM_KEY>
void test(
	const char *text,
	int nb,
	const std::vector<std::string> &allAttributes,
	const RANDOM_KEY &random_key )
{
	// test: lookup speed, insert speed, copy speed, memory
	std::cout << "==========================================" << std::endl;
	std::cout << text << " " << nb << std::endl;

	std::cout << "sizeof = " << sizeof(T) << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	{
		T container;
		while ( container.size() < nb )
		{
			container.emplace(
				random_key(),
				allAttributes[absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 )] );
		}
	}
	auto time = std::chrono::high_resolution_clock::now() - start;
	std::cout << "creation/destruction time = " << print( time ) << std::endl;

	std::vector<std::unique_ptr<char[]>> randomAllocs;
	T container;
	while ( container.size() < nb )
	{
		container.emplace(
			random_key(),
			allAttributes[absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 )] );
		randomizeHeap( randomAllocs );
	}
	randomAllocs.clear();

	start = std::chrono::high_resolution_clock::now();
	int found = 0;
	for ( int i = 0; i < 1000; ++i )
	{
		auto it = container.find( random_key() );
		if ( it != container.end() )
			++found;
	}

	time = std::chrono::high_resolution_clock::now() - start;
	std::cout << "lookup time = " << print( time ) << std::endl;
}

int main( int argc, char **argv )
{
	auto allAttributes = generateAllAttribs( 8, 64, 800'000 );

	auto random_string_key = [&]
	{
		return allAttributes[absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 )];
	};
	auto random_int_key = [&]
	{
		return absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 );
	};

	test<IndexedDict<std::string,10>>( "IndexedDict", 10, allAttributes, random_int_key );
	test<IndexedDict<std::string,100>>( "IndexedDict", 100, allAttributes, random_int_key );
	test<IndexedDict<std::string,1000>>( "IndexedDict", 1000, allAttributes, random_int_key );
	test<IndexedDict<std::string,100'000>>( "IndexedDict", 100'000, allAttributes, random_int_key );

	for ( auto nb : { 10, 100, 1000, 100'000 } )
	{
		test<std::map<std::string,std::string>>( "std::map", nb, allAttributes, random_string_key );
		test<std::map<int,std::string>>( "std::map2", nb, allAttributes, random_int_key );
		test<std::unordered_map<std::string,std::string>>( "std::unordered_map", nb, allAttributes, random_string_key );
		test<absl::btree_map<std::string,std::string>>( "btree_map", nb, allAttributes, random_string_key );
		test<absl::flat_hash_map<std::string,std::string>>( "flat_hash_map", nb, allAttributes, random_string_key );
	}

}
