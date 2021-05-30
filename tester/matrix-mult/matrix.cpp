#include<bits/stdc++.h>
using namespace std ;
int A[21][21], B[21][21], C[21][21] ;
int n1, m1, n2, m2 ;

int main() {
	scanf("%d%d", &n1, &m1) ;
	for ( int i = 1; i <= n1; ++i )
		for ( int j = 1; j <= m1; ++j )
			scanf("%d", &A[i][j]) ;
	
	scanf("%d%d", &n2, &m2) ;
	for ( int i = 1; i <= n2; ++i )
		for ( int j = 1; j <= m2; ++j )
			scanf("%d", &B[i][j]) ;

	if ( m1 != n2 ) {
		printf("Incompatible Dimensions\n") ;
		return 0 ;
	}

	for ( int i = 1; i <= n1; ++i )
		for ( int j = 1; j <= m2; ++j )
			C[i][j] = 0 ;

	for ( int i = 1; i <= n1; ++i )
		for ( int j = 1; j<= m1; ++j )
			for ( int k = 1; k <= m2; ++k )
				C[i][k] += A[i][j] * B[j][k] ;

	
	for ( int i = 1; i <= n1; ++i ){
		for ( int j = 1; j <= m2; ++j )
			printf("%10d", C[i][j]) ;
		printf("\n") ;
	}

	return 0 ;
}
