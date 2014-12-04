'use strict';

var moveNumber = 0;

function colorizeBoard(boardData) {
	for (var i = 0; i < 42; ++i) {
		var r = Math.floor(i / 7) + 1;
		var c = (i % 7) + 1;
	
		var el = $('.c4-row:nth-child(' + r + ') > span:nth-child(' + c + ')');
	
		if (boardData[i] === 0) {
			el.removeClass('p1');
			el.removeClass('p2');
		}
		else if (boardData[i] === 1) {
			el.addClass('p1');
			el.removeClass('p2');
		}
		else if (boardData[i] === 2) {
			el.removeClass('p1');
			el.addClass('p2');
		}
	}
}

function checkGameState() {
	$.get('/game_state', function(data) {
		colorizeBoard(data.board);
	
		moveNumber = data.moveNumber;
	
		if (data.activePlayer === 2) {
			$('#message-area').text('Your turn! Drop your chip in any open column.');
			$('#controls button').prop('disabled', false);
		}
		else {
			$('#message-area').text('It\'s their turn. Waiting for them to move...');
			$('#controls button').prop('disabled', true);
		}
	});
}

function requestDrop() {
	var column = $(this).val();
	
	$.post('/game_state', 'column=' + column + '&moveNumber=' + moveNumber);
	
	return false;
}

$(document).ready(function() {
	$('#controls button').click(requestDrop);
	checkGameState();
	window.setInterval(checkGameState, 1000);	
});