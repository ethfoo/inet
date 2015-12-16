package com.ethfoo.chat;

import io.netty.channel.Channel;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.channel.group.ChannelGroup;
import io.netty.channel.group.DefaultChannelGroup;
import io.netty.util.concurrent.GlobalEventExecutor;

public class ChatServerHandler extends SimpleChannelInboundHandler<String>{

	public static ChannelGroup channelGroup = new DefaultChannelGroup(GlobalEventExecutor.INSTANCE);
	
	@Override
	public void channelActive(ChannelHandlerContext ctx) throws Exception {
		
	}

	@Override
	public void channelInactive(ChannelHandlerContext ctx) throws Exception {
		super.channelInactive(ctx);
	}

	@Override
	public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause)
			throws Exception {
		cause.printStackTrace();
		ctx.close();
	}

	@Override
	public void handlerAdded(ChannelHandlerContext ctx) throws Exception {
		Channel incoming = ctx.channel();
		channelGroup.writeAndFlush("[SERVER] - " + incoming.remoteAddress() + "加入\n");
		channelGroup.add(ctx.channel());
	}

	@Override
	public void handlerRemoved(ChannelHandlerContext ctx) throws Exception {
		Channel incoming = ctx.channel();
		channelGroup.writeAndFlush("[SERVER] - " + incoming.remoteAddress() + "离开\n");
	}

	@Override
	protected void channelRead0(ChannelHandlerContext ctx, String s)
			throws Exception {

		Channel incoming = ctx.channel();
		for(Channel ch : channelGroup){
			if( ch != incoming){
				ch.writeAndFlush("[" + incoming.remoteAddress() + "]" + s + "\n");
			}else{
				ch.writeAndFlush("[you]" + s + "\n");
			}
		}
		
	}

}

