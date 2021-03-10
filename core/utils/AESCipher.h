#ifndef _AES_CIPHER_H_
#define _AES_CIPHER_H_
#include <stdlib.h>
namespace cipher
{
    class AesEcbCipher
    {
    public:
        AesEcbCipher( const unsigned char *key, bool use_cbc = false ) : mKey( key ), mState( NULL ), use_cbc( use_cbc ) {};

        int encode( const unsigned char *src, unsigned int src_len, unsigned char *dest, unsigned int &dest_len );
        int decode( const unsigned char *src, unsigned int src_len, unsigned char *dest, unsigned int &dest_len );
    private:
        typedef unsigned char state_t[4][4];
        unsigned char mRoundKey[240];
        const unsigned char *mKey;
        state_t *mState;
        bool use_cbc;
        static const unsigned char scSbox[256];
        static const unsigned char scRsbox[256];
        static const unsigned char scRcon[255];
        static const unsigned int KEYLEN;
        static const unsigned int NR;
        static const unsigned int NB;
        static const unsigned int NK;

        static inline unsigned char getSBoxValue( unsigned char num );
        static inline unsigned char getSBoxInvert( unsigned char num );
        static inline unsigned char xtime( unsigned char num );
        static inline unsigned char Multiply( unsigned char x, unsigned char y );

        void AddRoundKey( unsigned char round );
        void InvAddRoundKey( unsigned char round );
        void KeyExpansion();

        void MixColumns();
        void SubBytes();
        void ShiftRows();
        void Cipher();
        void AES128_ECB_encrypt( const unsigned char *input, unsigned char *out );

        void InvMixColumns();
        void InvSubBytes();
        void InvShiftRows();
        void InvCipher();
        void AES128_ECB_decrypt( const unsigned char *input, unsigned char *out );
    };
}


#endif
