#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FIRST(x) (((x) >> 12) & 0xF)
#define SECOND(x) (((x) >> 8) & 0xF)
#define THIRD(x) (((x) >> 4) & 0xF)
#define FOURTH(x) ((x) & 0xF)

#define GET_FIRST_BYTE(x) (((x) >> 8) & 0xFF)
#define GET_SECOND_BYTE(x) ((x) & 0xFF)

#define VX(x) (state.registers[SECOND((x))])
#define VY(y) (state.registers[THIRD((y))])
#define V(x) (state.registers[(x)])

#define D(x,y) ((state.display[((y) % 32)] >> ((x) % 64)) & 1)
//DS needs to be looked at and tested
#define DS(x,y,v) (state.display[((y) % 32)] ^= ((v) << ((x) % 8)))

int main( int, char** );
void run_instruction( uint16_t );


struct state{
	uint16_t pc;
	uint8_t mem[4096];
	
	uint8_t registers[16];
	uint16_t I;
	uint16_t stack[16];
	uint8_t sp;
	uint64_t display[32];
	uint8_t st, dt;
};

struct state state =
{0x200, {}, {}, 0, {}, 0, {}, 0, 0};

int main( int argc, char* argv[] )
{
	srandom(3);
	exit( EXIT_SUCCESS );
}

void run_instruction( uint16_t ins )
{
	//variables for 0xD instructions
	int yo,xo,d,m;
	//used for loops
	int i;

	switch( FIRST(ins) )
	{
	case 0:
		if( (ins & (~0xF)) == 0x00E0 )
		{
			if( FOURTH(ins) == 0xE ) //00EE
			{
				state.pc = state.stack[state.sp];
				state.sp--;
			} else //00E0
			{
				//zero out display
				memset( state.display, 0, 32 ); 
			}
			
		} else //0NNN
		{
			//call RCA 1802 at address NNN
			//rca( ins & 0xFFF );
		}
		break;
	case 1: //1NNN
		state.pc = (ins & 0xFFF); //unsure of this
		break;
	case 2: //2NNN
		//call function at NNN
		state.sp++;
		state.stack[state.sp] = state.pc;
		state.pc = (ins & 0xFFF);
		break;
	case 3: //3XNN
		if( VX(ins) == GET_SECOND_BYTE(ins) )
		{
			//skip next instruction
			state.pc += 2;
		}
		break;
	case 4: //4XNN
		if( VX(ins) != GET_SECOND_BYTE(ins) )
		{
			//skip next instruction
			state.pc += 2;
		}
		break;
	case 5: //5XY0
		if( VX(ins) == VY(ins) )
		{
			//skip next instruction
			state.pc += 2;
		}
		break;
	case 6: //6XNN
		VX(ins) = GET_SECOND_BYTE(ins);
		break;
	case 7: //7XNN
		VX(ins) += GET_SECOND_BYTE(ins);
		break;
	case 8: //8XY
		switch( FOURTH(ins) )
		{
		case 0:
			VX(ins) = VY(ins);
			break;
		case 1:
			VX(ins) |= VY(ins);
			break;
		case 2:
			VX(ins) &= VY(ins);
			break;
		case 3:
			VX(ins) ^= VY(ins);
			break;
		case 4:
			V(0xF) = (((uint16_t) VX(ins)) + ((uint16_t) VY(ins)) > 0xFF )? 1 : 0;
			VX(ins) += VY(ins);
			break;
		case 5:
			V(0xF) = (VX(ins) > VY(ins)) ? 1 : 0;
			VX(ins) -= VY(ins);
			break;
		case 6:
			V(0xF) = (VX(ins) & 0x1);
			VX(ins) >>= 1;
			break;
		case 7:
			V(0xF) = (VY(ins) > VX(ins))? 1 : 0;
			VX(ins) = VY(ins) - VX(ins);
			break;
		case 0xE:
			V(0xF) = ((VX(ins) >> 7) & 0x1);
			VX(ins) <<= 1;
			break;
		}
		break;
	case 9:
		if( VX(ins) != VY(ins) )
		{
			//skip next instruction
			state.pc += 2;
		}
		break;
	case 0xA:
		state.I = (ins & 0xFFF);
		break;
	case 0xB:
		//jump to (ins & 0xFFF) + V(0)
		state.pc = (ins & 0xFFF) + V(0);
		break;
	case 0xC:
		VX(ins) = (random() % 256) & (ins & 0xFFF);
		break;
	case 0xD:
		V(0xF) = 0;
		for( yo = 0; yo < FOURTH(ins); ++yo )
		{
			for( xo = 0; xo < 8; ++xo )
			{
				//unsure if this actually mutates the screen state
				d = D(VX(ins) + xo, VY(ins) + yo);	
				m = (state.mem[state.I] >> (7 - xo)) & 1;
				if( d && m )
					V(0xF) = 1;
				DS(VX(ins) + xo, VY(ins) + yo,m);
			}
		}
		break;
	case 0xE:
		if( GET_SECOND_BYTE(ins) == 0x9E )
		{
			if( 0/* VX(ins) == keypresseddown*/)
				state.pc += 2;
		} else if( GET_SECOND_BYTE(ins) == 0xA1 )
		{
			if( 0 /* ! VX(ins) == keypresseddown*/)
				state.pc += 2;
		}
		break;
	case 0xF:
		switch( GET_SECOND_BYTE(ins) )
		{
		case 0x07:
			VX(ins) = state.dt;
			break;
		case 0x0A:
			//wait for key press
			VX(ins) = 0/*key pressed down*/;
			break;
		case 0x15:
			state.dt = VX(ins);
			break;
		case 0x18:
			state.st = VX(ins);
			break;
		case 0x1E:
			state.I += VX(ins);
			break;
		case 0x29:
			//
			break;
		case 0x33:
			//
			break;
		case 0x55:
			for( i = 0; i < 16; ++i )
			{
				state.mem[(state.I + i)] = V(i);
			}
			break;
		case 0x65:
			for( i =0; i < 16; ++i )
			{
				V(i) = state.mem[(state.I + i)];
			}
			break;
		}
		break;
		
	}

}
