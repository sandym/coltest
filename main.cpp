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

template<typename T,int N>
class IndexedDict
{
private:
	std::bitset<N> _lookup;
	std::vector<T> _data;

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

template<typename T>
void test( const char *text, int nb, const std::vector<std::string> &allAttributes )
{
	// test: lookup speed, insert speed, copy speed, memory
	std::cout << "==========================================" << std::endl;
	std::cout << text << std::endl;

	std::cout << "sizeof = " << sizeof(T) << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	T container;
	while ( container.size() < nb )
	{
		container.emplace(
			allAttributes[absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 )],
			allAttributes[absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 )] );
	}
	auto time = std::chrono::high_resolution_clock::now() - start;
	std::cout << "creation time = " << print( time ) << std::endl;

	start = std::chrono::high_resolution_clock::now();
	int found = 0;
	for ( int i = 0; i < 1000; ++i )
	{
		auto it = container.find(allAttributes[absl::Uniform<size_t>( g_bitgen, 0, allAttributes.size() - 1 )]);
		if ( it != container.end() )
			++found;
	}

	time = std::chrono::high_resolution_clock::now() - start;
	std::cout << "lookup time = " << print( time ) << std::endl;
}

int main( int argc, char **argv )
{
	auto allAttributes = generateAllAttribs( 8, 64, 40000 );

	for ( auto nb : { 10, 100, 1000, 10000 } )
	{
		test<std::map<std::string,std::string>>( "std::map", nb, allAttributes );
		test<std::unordered_map<std::string,std::string>>( "std::unordered_map", nb, allAttributes );
		test<absl::btree_map<std::string,std::string>>( "btree_map", nb, allAttributes );
		test<absl::flat_hash_map<std::string,std::string>>( "flat_hash_map", nb, allAttributes );
//		test<IndexedDict<std::string,>>( nb, allAttributes );
	}
}
