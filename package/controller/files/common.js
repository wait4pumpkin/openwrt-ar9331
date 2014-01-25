var contentClientHeight=0;

function documentReady(){
	contentClientHeight=content.clientHeight;
	resetSize();
}

function addEvent(obj,event,fn){
	if ( obj.addEventListener ) {
		obj.addEventListener(event, fn, false );
	} else if ( obj.attachEvent ) {
		obj.attachEvent('on'+event, fn);
	}
}

function windowHeight()
{
	return document.documentElement.clientHeight;
}

function windowWidth()
{
	return document.documentElement.clientWidth;
}

function resetSize(){
	var maxwidth=509;
	var contentWidth=windowWidth();
	var contentHeight=windowHeight();
	
	var header=document.getElementById("header");
	var content=document.getElementById("content");
	var footer=document.getElementById("footer");

	if(header!=null)
		contentHeight=contentHeight-header.clientHeight;	
	if(footer!=null)
		contentHeight=contentHeight-footer.clientHeight;	
	if(contentHeight<contentClientHeight)
		contentHeight=contentClientHeight;
		
	content.style.height=contentHeight+"px";
	
	var width=contentWidth-20;
	if(width>maxwidth)
		width=maxwidth;
	var headerbody=document.getElementById("headerbody");
	var contentbody=document.getElementById("contentbody");
	if(headerbody!=null)
		headerbody.style.width=width+"px";
	contentbody.style.width=width+"px";
}

function writeFooter(mac){
	var text="<li>系统版本：XW0.10 MAC："+mac+"</li>"
			+"<li>服务电话：40060-24680</li>"
			+"<li>2013小微版权所有</li>";
	document.write(text);
}