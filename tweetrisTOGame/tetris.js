

(function() {
var tetris = {
        board:[],
        boardDiv:null,
        canvas:null,
        
        pSize:(isiPhone())?20:40,
        canvasHeight:(isiPhone())?440:880,
        canvasWidth:(isiPhone())?220:440,
        boardHeight:0,
        boardWidth:0,
        
        /*pSize:40,
        canvasHeight:880,
        canvasWidth:440,
        boardHeight:0,
        boardWidth:0,
        */
        spawnX:4,
        spawnY:1,
        shapes:[
            [
                [-1,1],[0,1],[1,1],[0,0] //TEE shift and flip
            ],
            [
                [-1,0],[0,0],[1,0],[2,0] //line shift left
            ],
            [
                [-1,-1],[-1,0],[0,0],[1,0] //L EL shift left up
            ],
            [
                [1,-1],[-1,0],[0,0],[1,0] //R EL
            ],
            [
                [0,-1],[1,-1],[-1,0],[0,0] //R ess
            ],
            [
                [-1,-1],[0,-1],[0,0],[1,0] // L ess
            ],
            [
                [0,-1],[1,-1],[0,0],[1,0] // square
            ]
        ],
        tempShapes:null,
        curShape:null,
        curShapeIndex:null,
        curX:0,
        curY:0,
        curSqs:[],
        nextShape:null,
        nextShapeDisplay:null,
        nextShapeIndex:null,
        sqs:[],
        score:0,
        scoreDisplay:null,
        level:1,
        levelDisplay:null,
        numLevels:10,
        time:0,
        maxTime:1000,
        timeDisplay:null,
        isActive:0,
        curComplete:false,
        timer:null,
        sTimer:null,
        speed:700,
        lines:0,
        
        // TWEETRIS
        blocqueue:[],
        lastTetromino:null,
        imageReady:false,
        maxfeeditems:10,
        
        log:function(msg){
            if(window["console"] && console["log"]){
                console.log(msg);
            }
        },
        init:function() {
        	// change canvas details if small device
        	/*
        	if(this.isiPhone ()) {
				pSize = 40;
	        	canvasHeight = 880;
    	    	canvasWidth = 440;
        	}
        	*/
            this.canvas = document.getElementById("canvas");
            this.initBoard();
            this.initInfo();
            this.initLevelScores();
            this.initShapes();
            this.bindKeyEvents();
            this.bindControlEvents();
            this.play();
            
        },
        initBoard:function() {
            this.boardHeight = this.canvasHeight/this.pSize;
            this.boardWidth = this.canvasWidth/this.pSize;
            var s = this.boardHeight * this.boardWidth;
            for (var i=0;i<s;i++) {
                this.board.push(0);
            }
            //this.boardDiv = document.getElementById('board'); // for debugging
        },
        initInfo:function() {
            this.nextShapeDisplay = document.getElementById("next_shape");
            this.levelDisplay = document.getElementById("level").getElementsByTagName("span")[0];
            this.timeDisplay = document.getElementById("time").getElementsByTagName("span")[0];
            this.scoreDisplay = document.getElementById("score").getElementsByTagName("span")[0];
            this.linesDisplay = document.getElementById("lines").getElementsByTagName("span")[0];
            this.setInfo('time');
            this.setInfo('score');
            this.setInfo('level');
            this.setInfo('lines');
        },
        initShapes:function() {
            this.curSqs = [];
            this.curComplete = false;
            this.shiftTempShapes();
            this.curShapeIndex = this.tempShapes[0];
            this.curShape = this.shapes[this.curShapeIndex];
            this.initNextShape();
            this.setCurCoords(this.spawnX,this.spawnY);
            this.refreshBlocqueue ();
            this.drawShapeTO(this.curX,this.curY,this.curShape);
        },
        initNextShape:function() {
            if (typeof this.tempShapes[1] === 'undefined') {this.initTempShapes();}
            try {
                this.nextShapeIndex = this.tempShapes[1];
                this.nextShape = this.shapes[this.nextShapeIndex];
                this.drawNextShape();
            } catch(e) {
                throw new Error("Could not create next shape. " + e);
            }
        },
        initTempShapes:function() {
            this.tempShapes = [];
            for (var i = 0;i<this.shapes.length;i++) {
                this.tempShapes.push(i);
            }
            var k = this.tempShapes.length;
            while ( --k ) { //Fisher Yates Shuffle
                var j = Math.floor( Math.random() * ( k + 1 ) );
                var tempk = this.tempShapes[k];
                var tempj = this.tempShapes[j];
                this.tempShapes[k] = tempj;
                this.tempShapes[j] = tempk;
            }
        },
        shiftTempShapes:function() {
            try {
                if (typeof this.tempShapes === 'undefined' || this.tempShapes === null) {
                    this.initTempShapes();
                } else {
                    this.tempShapes.shift();
                }
            } catch(e) {
                throw new Error("Could not shift or init tempShapes:  " + e);
            }
        },
        initTimer:function() {
                var me = this;
                var tLoop = function() {
                    me.incTime();
                    me.timer = setTimeout(tLoop,2000);
                };
                this.timer = setTimeout(tLoop,2000);
        },
        initLevelScores:function() {
            var c = 1;
            for (var i=1;i<=this.numLevels;i++) {
                this['level' + i] = [c * 1000,40*i,5*i]; // for next level, row score, p score, TODO: speed
                c = c + c;
            }
        },
        setInfo:function(el) {
            this[el + 'Display'].innerHTML = this[el];
        },
        drawNextShape:function() {
                var ns = [];
                for (var i=0;i<this.nextShape.length;i++) {
                    ns[i] = this.createSquare(this.nextShape[i][0] + 2,this.nextShape[i][1] + 2,this.nextShapeIndex);
                }
                this.nextShapeDisplay.innerHTML = '';
                for (var k=0;k<ns.length;k++) {
                    this.nextShapeDisplay.appendChild(ns[k]);
                }
        },
        drawShape:function(x,y,p) {
        
            for (var i=0;i<p.length;i++) {
                var newX = p[i][0] + x;
                var newY = p[i][1] + y;
                this.curSqs[i] = this.createSquare(newX,newY,this.curShapeIndex);
//                this.curSqs[i] = this.createSquareTO(newX,newY,i,p,this.curShapeIndex);
            }
            for (var k=0;k<this.curSqs.length;k++) {
                this.canvas.appendChild(this.curSqs[k]);
            }
        },
        createSquare:function(x,y,type) {
            var el = document.createElement('div');
            el.className = 'square type'+type;
//            el.style.backgroundImage = 'url(http://web.cs.dal.ca/~reilly/images/joe.jpg)';
            //el.style.backgroundImage='url("IMG_8540.jpg")';
            el.style.left = x * this.pSize + 'px';
            el.style.top = y * this.pSize + 'px';
            return el;
        },
        removeCur:function() {
            var me = this;
            this.curSqs.eachdo(function() {
                me.canvas.removeChild(this);
            });
            this.curSqs = [];
        },
        setCurCoords:function(x,y) {
            this.curX = x;
            this.curY = y;
        },
        bindKeyEvents:function() {
            var me = this;
            var event = "keypress";
            if (this.isSafari() || this.isIE()) {event = "keydown";}
            var cb = function(e) {
                me.handleKey(e);
            };
            if (window.addEventListener) {
                document.addEventListener(event, cb, false);
            } else {
                document.attachEvent('on' + event,cb);
            }
        },
        toggleControls: function(){
            var classes = this.controls.className.split(" "),
                cls_length = classes.length,
                hasClass = false,
                newClassList = [];
            this.log(classes);
            while(cls_length--){
                className = classes[cls_length];
                if(className == "show_controls"){
                    hasClass = true;
                } else {
                    newClassList.push(className);
                }
            }
            if(hasClass == false){
                newClassList.push("show_controls");
            }
            this.log(newClassList.join(" "));
            this.controls.className = newClassList.join(" ");
                
            
        },
        bindControlEvents:function() {
            var me = this,
                event = "click",
                cb = function(e) {
                    me.handleControl(e);
                },
                controls = document.getElementById("controls");
            this.controls = controls;
            if (window.addEventListener) {
                this.controls.addEventListener(event, cb, false);
            } else {
                this.controls.attachEvent('on' + event,cb);
            }
        },
        handleControl:function(e) {
            var classes = e.target.className.split(" "),
                cls_length = classes.length,
                dir = '';
            this.log(classes);
            while(cls_length--){
                class_name = classes[cls_length];
                switch (class_name) {
                    case "move_left":
                        this.move('L');
                        break;
                    case "move_rotate": // rotate
                        this.move('RT');
                        break;
                    case "move_right":
                        this.move('R');
                        break;
                    case "move_down":
                        this.move('D');
                        break;
                    case "move_pause": //esc:pause
                        this.togglePause();
                        break;
                    case "move_show_controls": //esc:pause
                        this.log("move_show_controls");
                        this.toggleControls();
                        break;
                    default:
                        break;
                }
            }
        },
        handleKey:function(e) {
            var c = this.whichKey(e);
            var dir = '';
            switch (c) {
                case 37:
                    this.move('L');
                    break;
                case 38: // rotate
                    this.move('RT');
                    break;
                case 39:
                    this.move('R');
                    break;
                case 40:
                    this.move('D');
                    break;
                case 27: //esc:pause
                    this.togglePause();
                    break;
                default:
                    break;
            }
        },
        whichKey:function(e) {
            var c;
            if (window.event) {c = window.event.keyCode;}
            else if (e) {c = e.keyCode;}
            return c;
        },
        incTime:function() {
            this.time++;
            this.setInfo('time');
        },
        incScore:function(amount) {
            this.score = this.score + amount;
            this.setInfo('score');
        },
        incLevel:function() {
            this.level++;
            this.speed = this.speed - 75;
            this.setInfo('level');
        },
        incLines:function(num) {
            this.lines += num;
            this.setInfo('lines');
        },
        calcScore:function(args) {
            var lines = args.lines || 0;
            var shape = args.shape || false;
            var speed = args.speed || 0;
            var score = 0;
            
            if (lines > 0) {
                score += lines*this["level" + this.level][1]; 
                this.incLines(lines);
            }
            if (shape === true) {score += shape*this["level"+this.level][2];}
            // if (speed > 0) {score += speed*this["level"+this.level[3]];} TODO: implement speed score
            this.incScore(score);
        },
        checkScore:function() {
            if (this.score >= this['level' + this.level][0]) {
                this.incLevel();
            }
        },
        gameOver:function() {
            this.clearTimers();
            this.canvas.innerHTML = "<h1>GAME OVER</h1>";
        },
        play:function() { //gameLoop
            var me = this;
            if (this.timer === null) {
                this.initTimer();
            }
            var gameLoop = function() {
                me.move('D');
                if(me.curComplete) {
                    me.markBoardShape(me.curX,me.curY,me.curShape);
                    me.curSqs.eachdo(function() {
                        me.sqs.push(this);
                    });
                    me.calcScore({shape:true});
                    me.checkRows();
                    me.checkScore();
                    me.initShapes();
                    me.play();
                } else {
                    me.pTimer = setTimeout(gameLoop,me.speed);
                }
            };
            this.pTimer = setTimeout(gameLoop,me.speed);
            this.isActive = 1;
        },
        togglePause:function() {
            if (this.isActive === 1) {
                this.clearTimers();
                this.isActive = 0;
            } else {this.play();} 
        },
        clearTimers:function() {
            clearTimeout(this.timer);
            clearTimeout(this.pTimer);
            this.timer = null;
            this.pTimer = null;
        },
        move:function(dir) {
            var s = '';
            var me = this;
            var tempX = this.curX;
            var tempY = this.curY;
            switch(dir) {
                case 'L':
                    s = 'left';
                    tempX -= 1;
                    break;
                case 'R':
                    s = 'left';
                    tempX += 1;
                    break;
                case 'D':
                    s = 'top';
                    tempY += 1;
                    break;
                case 'RT':
                    this.rotateTO();
                    return true;
                    break;
                default:
                    throw new Error('wtf');
                    break;       
            }
            if (this.checkMove(tempX,tempY,this.curShape)) {
                this.curSqs.eachdo(function(i) {
                    var l = parseInt(this.style[s],10);
                    dir === 'L' ? l-=me.pSize:l+=me.pSize;
                    this.style[s] = l + 'px';
                    return true;
                });
                this.curX = tempX;
                this.curY = tempY;
            } else if (dir === 'D') { //if move is invalid and down, piece must be complete
                if (this.curY === 1 || this.time === this.maxTime) {this.gameOver(); return false;}
                this.curComplete = true;
            }
            return true;
        },
        rotate:function() {
            if (this.curShapeIndex !== 6) { // if not the square
                var temp = [];
                this.curShape.eachdo(function() {
                    temp.push([this[1] * -1,this[0]]); // (-y,x)
                });
                if (this.checkMove(this.curX,this.curY,temp)) {
                    this.curShape = temp;
                    this.removeCur();
                    this.drawShape(this.curX,this.curY,this.curShape);
                } else { throw new Error("Could not rotate!");}
            }
        },
        checkMove:function(x,y,p) {
            if (this.isOB(x,y,p) || this.isCollision(x,y,p)) {return false;}
            return true;
        },
        isCollision:function(x,y,p) {
            var me = this;
            var bool = false;
            p.eachdo(function() {
                var newX = this[0] + x;
                var newY = this[1] + y;
                if (me.boardPos(newX,newY) === 1) {bool = true;}
            });
            return bool;
        },
        isOB:function(x,y,p) { 
            var w = this.boardWidth - 1;
            var h = this.boardHeight - 1;
            var bool = false;
            p.eachdo(function() {
                var newX = this[0] + x;
                var newY = this[1] + y;
                if(newX < 0 || newX > w || newY < 0 || newY > h) {bool = true;}
            });
            return bool;
        },
        getRowState:function(y) { //Empty, Full, or Used
            var c = 0;
            for (var x=0;x<this.boardWidth;x++) {
                if (this.boardPos(x,y) === 1) {c = c + 1;}
            }
            if (c === 0) {return 'E';}
            if (c === this.boardWidth) {return 'F';}
            return 'U';
        },
        checkRows:function() { //does check for full lines, removes them, and shifts everything else down
            /*var me = this;
            var memo = 0;
            var checks = (function() {
                    me.curShape.eachdo(function() {
                        if ((this[1] + me.curY) > memo) {
                            return this[1];
                        }
                    });                                     
            })();
            
            console.log(checks);*/
            
            
            var me = this;
            var start = this.boardHeight;
            this.curShape.eachdo(function() {
                var n = this[1] + me.curY;
                //this.log(n);
                if (n < start) {start = n;}
            });
            this.log(start);

            

            var c = 0;
            var stopCheck = false;
            for (var y=this.boardHeight - 1;y>=0;y--) {
                    switch(this.getRowState(y)) {
                        case 'F':
                            this.removeRow(y);
                            c++;
                            break;
                        case 'E':
                            if (c === 0) {  
                                stopCheck = true;
                            }
                            break;
                        case 'U':
                            if (c > 0) {
                                this.shiftRow(y,c);
                            }
                            break;
                        default:
                            break;
                    }
                    if (stopCheck === true) {
                        break;
                    }
            }
            if (c > 0) {
                this.calcScore({lines:c});
            }
        },
        shiftRow:function(y,amount) {
            var me = this;
            for (var x=0;x<this.boardWidth;x++) {
                this.sqs.eachdo(function() {
                    if (me.isAt(x,y,this)) {
                        me.setBlock(x,y+amount,this);
                    }
                });
            }
            me.emptyBoardRow(y);
        },
        emptyBoardRow:function(y) { // empties a row in the board array
            for (var x=0;x<this.boardWidth;x++) {
                this.markBoardAt(x,y,0);
            }
        },
        removeRow:function(y) {
            for (var x=0;x<this.boardWidth;x++) {
                this.removeBlock(x,y);
            }
        },
        removeBlock:function(x,y) {
            var me = this;
            this.markBoardAt(x,y,0);
            this.sqs.eachdo(function(i) {
                if (me.getPos(this)[0] === x && me.getPos(this)[1] === y) {
                    me.canvas.removeChild(this);
                    me.sqs.splice(i,1);
                }
            });
        },
        setBlock:function(x,y,block) {
            this.markBoardAt(x,y,1);
            var newX = x * this.pSize;
            var newY = y * this.pSize;
            block.style.left = newX + 'px';
            block.style.top = newY + 'px';
        },
        isAt:function(x,y,block) { // is given block at x,y?
            if(this.getPos(block)[0] === x && this.getPos(block)[1] === y) {return true;}
            return false;
        },
        getPos:function(block) { // returns [x,y] block position
            var p = [];
            p.push(parseInt(block.style.left,10)/this.pSize);
            p.push(parseInt(block.style.top,10)/this.pSize);
            return p;
        },
        getBoardIdx:function(x,y) { // returns board array index for x,y coords
            return x + (y*this.boardWidth);
        },
        boardPos:function(x,y) { // returns value at this board position
            return this.board[x+(y*this.boardWidth)];
        },
        markBoardAt:function(x,y,val) {
            this.board[this.getBoardIdx(x,y)] = val;
        },
        markBoardShape:function(x,y,p) {
            var me = this;
            p.eachdo(function(i) {
                var newX = p[i][0] + x;
                var newY = p[i][1] + y;
                me.markBoardAt(newX,newY,1);
            });
        },
        isIE:function() {
            return this.bTest(/IE/);
        },
        isFirefox:function() {
            return this.bTest(/Firefox/);
        },
        isSafari:function() {
            return this.bTest(/Safari/);
        },
        bTest:function(rgx) {
            return rgx.test(navigator.userAgent);
        },
        requestBrick:function(brickid) {
        },
        /*
        getTweets:function() {
            var newTetromino = null;
            var textStr = null;
            // Get latest tweets from TweetrisTO
            $.jTwitter('tweetrisTO', 10, function(data){
                $.each(data, function(i, post){
                    // make sure it's a block tweet
                    textStr = post.text;
                    if (textStr.search ("BLOCK") !== -1) { // a block post
                        // split into fields
                        var blockItems = textStr.match (",");
                        // format BLOCK,ID,TYPE,ROW,COL,ROTATION,IMGURL
                        // have we already processed this shape?
                        // if the block id is > the last one in the queue
                        if (this.lastTetromino !== null && this.lastTetromino.id <= blockItems[1]) { // new shape
                            // already working on this shape?
                            if (newTetromino !== null && newTetromino.id === blockItems[1]) {
                                if (!newTetromino.hasBlock (blockItems[3],blockItems[4])) {
                                    newTetromino.addBlock (blockItems[3], blockItems[4]);
                                    if (newTetromino.isComplete ()) blockqueue.add (newTetromino);
                                }
                            } else { // first block for this shape
                                newTetromino = new tetris.Tetromino (blockItems[1],blockItems[2],blockItems[5]);
                                newTetromino.addBlock (blockItems[3],blockItems[4],blockItems[6]);
                            }    
                        } // else we've already processed that shape

                    } // else ignore, not a shape tweet                    
                });
            });
        },
        */
        hasBlock:function (row, col) {
            return (this.blocks[row][col] === null);
        },
        isComplete:function () {
            var complete = false;
            switch (this.type) {
            case "TEE": 
                complete = (this.blocks[1][3] !== this.emptyImg && this.blocks[2][3] !== this.emptyImg &&
                            this.blocks[3][3] !== this.emptyImg && this.blocks[2][2] !== this.emptyImg);
                break;
            case "LINE":
                complete = (this.blocks[1][2] !== this.emptyImg && this.blocks[2][2] !== this.emptyImg &&
                            this.blocks[3][2] !== this.emptyImg && this.blocks[4][2] !== this.emptyImg);
                break;
            case "LEL":
                complete = (this.blocks[1][1] !== this.emptyImg && this.blocks[1][2] !== this.emptyImg &&
                            this.blocks[2][2] !== this.emptyImg && this.blocks[3][2] !== this.emptyImg);
                break;
            case "REL":
                complete = (this.blocks[3][1] !== this.emptyImg && this.blocks[1][2] !== this.emptyImg &&
                            this.blocks[2][2] !== this.emptyImg && this.blocks[3][2] !== this.emptyImg);
                break;
            case "RESS":
                complete = (this.blocks[2][1] !== this.emptyImg && this.blocks[3][1] !== this.emptyImg &&
                            this.blocks[1][2] !== this.emptyImg && this.blocks[2][2] !== this.emptyImg);
                break;
            case "LESS":
                complete = (this.blocks[1][1] !== this.emptyImg && this.blocks[2][1] !== this.emptyImg &&
                            this.blocks[2][2] !== this.emptyImg && this.blocks[3][2] !== this.emptyImg);
                break;
            case "SQUARE":
                complete = (this.blocks[2][1] !== this.emptyImg && this.blocks[3][1] !== this.emptyImg &&
                            this.blocks[2][2] !== this.emptyImg && this.blocks[3][2] !== this.emptyImg);
                break;
            default:
                break;
            }
            return complete;
        },
        getShapeCoords:function (type) {
            var shapeCoords = null;
            switch (type) {
            case "TEE": 
                shapeCoords = [[-1,1],[0,1],[1,1],[0,0]];
                break;
            case "LINE":
            	shapeCoords = [[-1,0],[0,0],[1,0],[2,0]];
                break;
            case "LEL":
            	shapeCoords = [[-1,-1],[-1,0],[0,0],[1,0]];
                break;
            case "REL":
            	shapeCoords = [[1,-1],[-1,0],[0,0],[1,0]];
                break;
            case "RESS":
            	shapeCoords = [[0,-1],[1,-1],[-1,0],[0,0]];
                break;
            case "LESS":
            	shapeCoords = [[-1,-1],[0,-1],[0,0],[1,0]];
                break;
            case "SQUARE":
            	shapeCoords = [[0,-1],[1,-1],[0,0],[1,0]];
                break;
            default:
            	throw new Error ("got wacky type: " + type);
                break;
            }
            return shapeCoords;
        },
        getTypeIndex:function () {
            var typeIndex =-1;
            switch (this.type) {
            case "TEE": typeIndex = 0;
                break;
            case "LINE": typeIndex = 1;
                break;
            case "LEL": typeIndex = 2;
                break;
            case "REL": typeIndex = 3;
                break;
            case "RESS": typeIndex = 4;
                break;
            case "LESS": typeIndex = 5;
                break;
            case "SQUARE": typeIndex = 6;
                break;
            default:
            	throw new Error ("got wacky type: " + type);
                break;
            }
            return typeIndex;
        },
         addBlock:function (row, col, imgURL) {
            // add to blocks
            this.blocks[row][col] = imgURL;
        },
        getImgURL:function (row, col) { return this.blocks[row][col]; },
        
        Tetromino:function (id,type,rotation) {
            this.id = id;
            this.type = type;
            this.rotation = rotation;
            this.emptyImg = "IMG_8540.jpg"
            this.blocks = new Array (5);
            for (var i = 0; i < this.blocks.length; i++) {
            	this.blocks[i] = new Array (5);
            	for (var j = 0; j < this.blocks[i].length; j++) {
            		this.blocks[i][j] = this.emptyImg;
            	}
            }
            this.hasBlock = tetris.hasBlock;
            this.addBlock = tetris.addBlock;
            this.isComplete = tetris.isComplete;
            this.getImgURL = tetris.getImgURL;
            this.getTypeIndex = tetris.getTypeIndex;
        },
        createSquareTO:function(x,y,i,p,type) {
            var el = document.createElement('div');
            el.className = 'square type'+type;
            el.style.left = x * this.pSize + 'px';
            el.style.top = y * this.pSize + 'px';
            var elRow = p[i][0]+2;
            var elCol = p[i][1]+2;
            var elURL = this.lastTetromino.blocks[elRow][elCol]; 
            el.style.backgroundImage = 'url(' + elURL + ')';
            el.style.backgroundSize = 'contain';
            //el.style.transform = 'rotate(' + this.lastTetromino.rotation + ')';
            var rotation = this.lastTetromino.rotation;
            if (rotation < 0) rotation = 360 + rotation;
 //           el.style.rotation = '' + rotation + 'deg';
			$(el).css({
    			'transform': 'rotate(' + rotation + 'deg)',
    			'-moz-transform': 'rotate(' + rotation + 'deg)',
    			'-o-transform': 'rotate(' + rotation + 'deg)',
    			'-webkit-transform': 'rotate(' + rotation + 'deg)'
			});            
            
            
            return el;
        },
        refreshBlocqueue:function () {
        	            // refresh the blocqueue if necessary
            if (this.blocqueue.length === 0) this.refreshFeed ();
            // pull the tetromino to use
            if (this.blocqueue.length !== 0) {
                this.lastTetromino = this.blocqueue.pop ();
                // replace curShape with the new shape coordinate data
                this.curShape = this.getShapeCoords (this.lastTetromino.type);
                this.imageReady = true;
            } else {
                this.imageReady = false;
            }

        },
        drawShapeTO:function(x,y,p) {
            for (var i=0;i<p.length;i++) {
                var newX = p[i][0] + x;
                var newY = p[i][1] + y;
                if (this.imageReady) {
                	this.curSqs [i] = this.createSquareTO (newX,newY,i,p,this.lastTetromino.getTypeIndex ());
                } else {
                    this.curSqs [i] = this.createSquare (newX,newY,this.curShapeIndex);
                }
            }
            for (var k=0;k<this.curSqs.length;k++) {
                this.canvas.appendChild(this.curSqs[k]);
            }
        },
        rotateTO:function() {
            if (this.imageReady) {
                var temp = [];
                this.curShape.eachdo(function () {
                    temp.push([this[1] * -1,this[0]]); // (-y,x)
                });
                if (this.checkMove(this.curX,this.curY,temp)) { 
	                // update the tetromino block array to reflect the new rotation
                	for (var i = 0; i < temp.length; i++) { 
                		var stow = this.lastTetromino.blocks[temp[i][0]+2][temp[i][1]+2]; 
                		this.lastTetromino.blocks[temp[i][0]+2][temp[i][1]+2] = 
                			this.lastTetromino.blocks[this.curShape[i][0]+2][this.curShape[i][1]+2]; 
                		this.lastTetromino.blocks[this.curShape[i][0]+2][this.curShape[i][1]+2] = stow;
                	}
                    this.curShape = temp;
	                this.lastTetromino.rotation -= 90;
                    this.removeCur();
                    this.drawShapeTO(this.curX,this.curY,this.curShape);
                } else { throw new Error("Could not rotate!");}
            } else { // using regular shapes
                this.rotate ();
            }
            
        },
        

	refreshSpecificFeed:function ( feedurl, maxitems ) {	
		// Cap off number of items from any feed to prevent saturation unless configured
		if ( maxitems > this.maxfeeditems )
			maxitems = this.maxfeeditems;
	
 			var T = this;
		$.jGFeed ( feedurl, function(feeds) {
            var newTetromino = null;
				
		  //sourceimages = new Array();
			
	  		if (!feeds) {
	    		return ( false );	
	  		}
	  
			for ( var i=0; i<feeds.entries.length; i++ ) {
			  var entry = feeds.entries[i];
			  var textStr = entry.title.replace(entry.link, '' );
			  var url = 'http://www.twitpic.com/show/thumb/' + entry.link.replace ('http://twitpic.com/', '');
			  
    	      if (textStr.search ("BLOCK") !== -1) { // a block post
        	      // split into fields
          		var blockItems = textStr.split (",");
            	// format BLOCK,ID,TYPE,ROW,COL,ROTATION,IMGURL
            	// have we already processed this shape?
            	// if the block id is > the last one in the queue
            	var processThis = (T.blocqueue.length === 0) ? 
            		((T.lastTetromino === null) ? true : ((T.lastTetromino.id < blockItems[1]) ? true : false))
            		: (T.blocqueue[T.blocqueue.length-1].id < blockItems[1]) ? true: false;
            	if (processThis) { // new shape
                	// already working on this shape?
                	if (newTetromino !== null && newTetromino.id === blockItems[1]) {
                    	if (!newTetromino.hasBlock (parseInt(blockItems[4])+1,parseInt(blockItems[5])+1)) {
                        	newTetromino.addBlock (parseInt(blockItems[4])+1, parseInt(blockItems[5])+1, url);
                        	if (newTetromino.isComplete ()) T.blocqueue.push (newTetromino);
                    	}
                	} else { // first block for this shape
                		// notice that an incomplete shape will be garbage collected. We assume that the 
                		// feed provides all 4 blocks for a particular shape in an uninterrupted sequence
                    	newTetromino = new tetris.Tetromino (blockItems[1],blockItems[2],blockItems[3]);
                    	newTetromino.addBlock (parseInt(blockItems[4])+1,parseInt(blockItems[5])+1,url);
                    	tetris.log ("added block with id " + blockItems[1] + ", type " + blockItems[2]);
                	}    
            	} // else we've already processed that shape

	          } // else ignore, not a shape tweet                    
			  
		  
	      }
			
			
		}, maxitems );
	},
        
 	//
	// Load the twitpic public timeline
	//
	refreshFeed:function()	{
 	   	this.refreshSpecificFeed ('http://twitpic.com/photos/nuitblanchebloc/feed.rss', 20);
	},
       
        /*debug:function() {
            var me = this;
            var str = '';
            for (var i=0;i<me.board.length;i++) {
                if(i%me.boardWidth === 0) {str += "<br />"}
                if(me.board[i] === 1) {str += ' X ';}
                else {str += "&nbsp;*&nbsp;";}
            }
            var par = document.createElement('p');
            par.innerHTML = str;
            me.boardDiv.innerHTML = '';
            me.boardDiv.appendChild(par);
        },*/
};
tetris.init();
})();

if (!Array.prototype.eachdo) {
    Array.prototype.eachdo = function(fn) {
        for (var i = 0;i<this.length;i++) {
            fn.call(this[i],i);
        }
    };
}

if (!Array.prototype.remDup) {
    Array.prototype.remDup = function() {
        var temp = [];
        for(var i=0; i<this.length; i++) {
          var bool = true;
            for(var j=i+1; j<this.length; j++) {
                if(this[i] === this[j]) {bool = false;}     
            }   
            if(bool === true) {temp.push(this[i]);}
        }
        return temp;
    }
}