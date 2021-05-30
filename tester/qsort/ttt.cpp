#include<bits/stdc++.h>
using namespace std ;

int n ;
int a[100010] ;

void _qsort(int l, int r) {
	// printf("( %d, %d ):\n", l, r) ;
	// getchar() ;
	if ( l >= r )	return ;
	int pivot = a[l] ;
	// printf("pivot = %d\n", pivot) ;
	int ll = l, rr = r ;
	while ( ll < rr ) {
		while ( a[ll] < pivot)
			++ ll ;
		while ( a[rr] > pivot)
			-- rr ;
		// printf("ll = %d, rr = %d\n", ll, rr) ;

		if ( ll <= rr ){
			if ( a[ll] >= a[rr])
				swap(a[ll], a[rr]) ;
			++ll, --rr ;
		}
	}
	_qsort(l, rr) ;
	_qsort(ll, r) ;
}

vector<int> out ;
int main(){
	scanf("%d", &n) ;
	for ( int i = 1; i <= n; ++i )
		scanf("%d", a+i) ;


	_qsort(1, n) ;

	for ( int i = 1; i <= n; ++i ){
		// if ( a[i] % 2 )
		//	out.push_back(a[i]) ;
		printf("%d\n", a[i]) ;
	}


	/*
	for ( int i = 0; i < out.size(); ++i )
		printf("%d%c", out[i], i == out.size()-1 ? '\n':',') ;

	return 0 ;
*/
}
