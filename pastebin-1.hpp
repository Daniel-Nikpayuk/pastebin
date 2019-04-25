/************************************************************************************************************************
**
** Copyright 2019 Daniel Nikpayuk
**
************************************************************************************************************************/

	#include<iostream>

// convenience definitions for this demonstration:

	using size_type = unsigned;

	template<typename Type, Type Value>
	struct memoized_value
	{
		static constexpr Type value = Value;
	};

// recursive continuation passing definition, as well as a way ("closure_at") to access the lexical closure:

	template<bool True, typename Filler = void>
	struct recurse
	{
		template<size_type index, typename Value, typename... Values>
		using closure_at = typename recurse<bool(index > 1)>::template closure_at<index-1, Values...>;

		template
		<
			size_type index, template<size_type> class ContList,
			typename Type, template<Type...> class ListType, Type... Values

		> using continuation = typename ContList<index>::template apply<index-1, ContList, Type, ListType, Values...>;
	};

	template<typename Filler>
	struct recurse<false, Filler>
	{
		template<size_type index, typename Value, typename... Values>
		using closure_at = Value;

		template
		<
			size_type index, template<size_type> class ContList,
			typename Type, template<Type...> class ListType, Type... Values

		> using continuation = ListType<Values...>;
	};

// main operators defined for continuation passing:

	struct Cons
	{
		template
		<
			size_type index, template<size_type> class ContList,
			typename Type, template<Type...> class ListType, Type... Values

		> using apply = typename recurse<bool(index)>::template continuation<index, ContList, Type, ListType, Values...>;
	};

	struct Car
	{
		template
		<
			size_type index, template<size_type> class ContList,
			typename Type, template<Type...> class ListType, Type Value, Type... Values

		> using apply = typename recurse<bool(index)>::template continuation<index, ContList, Type, ListType, Value>;
	};

	struct Cdr
	{
		template
		<
			size_type index, template<size_type> class ContList,
			typename Type, template<Type...> class ListType, Type Value, Type... Values

		> using apply = typename recurse<bool(index)>::template continuation<index, ContList, Type, ListType, Values...>;
	};

// regular car definition:

	struct structure
	{
		template<typename Type, template<Type...> class ListType, Type Value, Type... Values>
		using car = memoized_value<Type, Value>;
	};

// list unpacking. 'push_front' is used for "regular car", 'apply' is used for continuation passing:

	template<typename, typename> struct unpack_list;

	template<typename Type, Type... Values, template<Type...> class ListType>
	struct unpack_list<Type, ListType<Values...>>
	{
		template
		<
			template<typename Kind, template<Kind...> class ListKind, Kind...> class signature,
			Type... Args

				//   signature: cons, multicons, car, cdr, null, length.

		> using push_front = signature<Type, ListType, Args..., Values...>;

		template
		<
			// initialize index as the last element of the ContList (length-1)

			size_type index, template<size_type> class ContList
		>
		using apply = typename recurse<bool(index)>::template continuation
		<
			index, ContList, Type, ListType, Values...
		>;
	};


// hardcoded (lexically closed) list of functions to compose in continuation passing style <Car, Cdr, Cdr>(List)
// order of composition is right to left like math notation f(g(h(x))):


	template<size_type index>
	using composition = typename recurse<true>::template closure_at<index, Car, Cdr, Cdr, Cdr>;


// main demonstration:

	template<int...> struct int_list { };


int main(int argc, char *argv[])
{
	using list1 = int_list<5, 2, -1, 3>;

	using result = typename unpack_list<int, list1>::template apply<3, composition>;

	using number = typename unpack_list<int, result>::template push_front<structure::template car>;

	std::cout << number::value << "\n";

	return 0;
}

/*
	This approach does not scale, but it should reduce the memoization of what would otherwise be a list <Car, Cdr, Cdr>,
	as well as its "cdr" sublists <Car, Cdr>, <Car>. The original continuation passing style Cdr<Cdr<Car>>
	effectively memoizes as much and so shouldn't be any better, though I haven't run tests on this.

	As for scalability, given that most applications I can think of require the user to know which functions she wants
	to compose in advance, and if this approach really does save, it's worth the small amount of effort to hardcode
	(lexically close) a few lists.
*/

