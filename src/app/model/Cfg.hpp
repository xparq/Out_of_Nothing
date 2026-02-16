#ifndef _CVBD70MM479T6ND39F567MTNNH587D45F8_
#define _CVBD70MM479T6ND39F567MTNNH587D45F8_

namespace OON::Model::Cfg {

	using BasicNumberType = double; //!! Shockingly, double was faster than float: ~160 FPS vs. 130! :-o
	                                //!! I double-checked once (pun intended...), but couldn't repr. later!
					//!! (It was not consistently faster with Seconds = float or double either.)
	                                //!! Could be an alignment issue!
}

#endif // _CVBD70MM479T6ND39F567MTNNH587D45F8_
