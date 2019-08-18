	.globl  KmodGetkmodapiStart
	.globl  KmodGetkmodapiEnd

	.ent	KmodGetkmodapiStart
KmodGetkmodapiStart:
		lui	$2, %hi(LD_kmodapi_start)
		addiu	$2, $2, %lo(LD_kmodapi_start)
		jr	$ra
	.end	KmodGetkmodapiStart

	.ent	KmodGetkmodapiEnd

KmodGetkmodapiEnd:
		lui	$2, %hi(LD_kmodapi_end)
		addiu	$2, $2, %lo(LD_kmodapi_end)
		jr	$ra
	.end	KmodGetkmodapiEnd
