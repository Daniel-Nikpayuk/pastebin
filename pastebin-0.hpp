/************************************************************************************************************************
**
** Copyright 2019 Daniel Nikpayuk
**
************************************************************************************************************************/

/*
	Memoization is a sunk cost when you need to pattern match.

	To reduce memoization, the general strategy is to figure out the atomic operations for a pattern,
	implement them efficiently, then implement all other operations (for that pattern) as combinations
	of those atomic operations.

	This approach can be improved further: Instead of packaging those atomic operations directly
	with the memoized structs, we can refactor by packaging only their operation signatures.

	For example, cons, multicons, car, cdr, null, length all use the following signature:

		signature<Type, ListType, Args..., Values...>

	which is accessed through the "push_front" alias packaged with the memoized_list (below).

	*note* --- If you're unfamiliar with "cons", "car", "cdr" they're traditional lisp functional programming operators.
*/

	// default:

	template<typename, typename>
	struct memoized_list
	{
		using rtn = memoized_list;

		// identify:

		template
		<
			template<bool> class signature

			//      signature: is_list.

		> using match = signature<false>;
	};

	// matched pattern:

	template<typename Type, Type... Values, template<Type...> class ListType>
	struct memoized_list<Type, ListType<Values...>>
	{
		using rtn = memoized_list;

		// identify:

		template
		<
			template<bool> class signature

			//      signature: is_list.

		> using match = signature<true>;

		// grow:

		template
		<
			template<typename Kind, template<Kind...> class ListKind, Kind...> class signature,
			Type... Args

				//   signature: cons, multicons, car, cdr, null, length.

		> using push_front = signature<Type, ListType, Args..., Values...>;

		template
		<
			template<typename Kind, typename Object, Kind...> class signature,
			typename List, Type... Args

				//   signature: catenate.

		> using join_front = signature<Type, List, Args..., Values...>;

		// mutate:

			//  no signature, but required for zip (signature).

		template<typename Kind, template<Kind...> class ListKind, typename Op, Type... Args>
		using bimap = ListKind<Op::value(Args, Values)...>;

		template
		<
			template
			<
				typename Kind0, template<Kind0...> class ListKind0,
				typename Kind1, typename Func, typename Object, Kind1...

			> class signature,

			typename Kind, template<Kind...> class ListKind,
			typename Op, typename List

			//    signature: zip.

		> using zip = signature<Kind, ListKind, Type, Op, List, Values...>;

		// shrink:
	};

/*
	*note* --- I copied and pasted this code from my personal library, simplified and cleaned it up for this presentation.
		   There might be a few missing details, but otherwise it should be straightforward.

	Now we can define the actual operators to match the given signatures:
*/

	struct structure
	{
		template<bool Value>
		using is_list = memoized_value<bool, Value>;

		template<typename Type, template<Type...> class ListType, Type... Values>
		using cons = ListType<Values...>;

		template<typename Type, template<Type...> class ListType, Type Value, Type... Values>
		using car = memoized_value<Type, Value>;

		template<typename Type, template<Type...> class ListType, Type Value, Type... Values>
		using cdr = ListType<Values...>;

		template<typename Type, typename List, Type... Values>
		using catenate = typename memoized_list<Type, List>::template push_front<cons, Values...>;

		template<typename Kind, template<Kind...> class ListKind, typename Type, typename Op, typename List, Type... Values>
		using zip = typename memoized_list<Type, List>::template bimap<Kind, ListKind, Op, Values...>;
	};

/*
	Now we implement more user friendly versions:
*/

/*
	cons:
*/

	template<typename Type, Type Value, typename List>
	using list_cons = typename memoized_list<Type, List>::template push_front
	<
		structure::template cons, Value
	>;

/*
	car:
*/

	template<typename Type, typename List>
	using list_car = typename memoized_list<Type, List>::template push_front
	<
		structure::template car
	>;

/*
	cdr:
*/

	template<typename Type, typename List>
	using list_cdr = typename memoized_list<Type, List>::template push_front
	<
		structure::template cdr
	>;

/*
	catenate:
*/

	template<typename Type, typename List1, typename List2>
	using list_catenate = typename memoized_list<Type, List1>::template join_front
	<
		structure::template catenate, List2
	>;

/*
	packwise:

	To make Op more general, it doesn't assume the output type is the same as the input type, so both must be specified.
*/

	template<typename Kind, template<Kind...> class ListKind, typename Type, typename Op, typename List1, typename List2>
	using list_packwise = typename memoized_list<Type, List1>::template zip
	<
		structure::template zip,

		Kind, ListKind,

		Op,

		List2
	>;

/*
	Finally, we end with an example, but first its setup:
*/

	template<int...>				struct list			{ };
	template<char, typename = void>			struct single_char_binary;

	#define declare_single_char_binary(return_type, op_char, op_name)					\
														\
	template<typename Filler>										\
	struct single_char_binary<op_char, Filler>								\
	{													\
		template<typename Type>										\
		static constexpr return_type value(const Type Value1, const Type Value2)			\
		{												\
			return (Value1 op_name Value2);								\
		}												\
	};

	declare_single_char_binary(Type, '+', +)
	declare_single_char_binary(Type, '-', -)
	declare_single_char_binary(Type, '*', *)
	declare_single_char_binary(Type, '/', /)
	declare_single_char_binary(Type, '%', %)

/*
	Finally, an example. Here we're treating these lists as vectors and performing vector addition.
	This uses the memoization pattern I was asking about on Twitter.
*/

	using list1 = list<5, 3, 7, 8>;
	using list2 = list<1, 6, 1, 5>;

	using result = list_packwise<int, list, int, single_char_binary<'+'>, list1, list2>;

