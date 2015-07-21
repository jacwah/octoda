# octoda
*A disassembler for CHIP-8 programs*

Octoda [octo-dee-ay] is a tool for people who want to understand CHIP-8 programs.
I've used it on some of David Winter's CHIP-8 games found at [his site]
(http://www.pong-story.com/chip8/). Octoda is distributed under the GNU GPL v2
license. For more info, see LICENSE.

### Features
- Disassemble any CHIP-8 program quick as a wink
- Automatically determine what is code and what is data
- Full opcode reference included

## Usage
Use the `8da` command with the program file as argument.

```
$ 8da GAMES/MAZE
addr: [code]  inst  arg1, arg2
-------------------------------
0200: [A21E]  LDI   21E
0202: [C201]  RND   V2, 1
0204: [3201]  SE    V2, 1
0206: [A21A]  LDI   21A
0208: [D014]  DRW   V0, V1, 4
020A: [7004]  ADD   V0, 4
020C: [3040]  SE    V0, 40
020E: [1200]  JMP   200
0210: [6000]  LD    V0, 0
0212: [7104]  ADD   V1, 4
0214: [3120]  SE    V1, 20
0216: [1200]  JMP   200
0218: [1218]  JMP   218
021A: [8040]  DATA  
021C: [2010]  DATA  
021E: [2040]  DATA  
0220: [8010]  DATA  
```

### How to compile

Octoda uses GNU Make.

```
$ make
$ sudo make install
```
