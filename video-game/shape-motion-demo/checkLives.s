.data
	
.text
	.global checkLives
	.type	checkLives, %function
	
checkLives:
	cmp #0, r12		;Checks how many lives are left
	jne END
	mov #1, r12		;If more than 0 lives, return 1, continue Game
	ret
END:
	mov #0, r12		;If o lives, end Game
	ret
	
