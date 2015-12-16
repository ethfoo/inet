package com.ethfoo.chat;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelInitializer;
import io.netty.channel.ChannelPipeline;
import io.netty.channel.EventLoopGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.DelimiterBasedFrameDecoder;
import io.netty.handler.codec.Delimiters;
import io.netty.handler.codec.string.StringDecoder;
import io.netty.handler.codec.string.StringEncoder;

public class ChatClient {

	private final String host;
	private final int port;
	
	public ChatClient(String host, int port){
		this.host = host;
		this.port = port;
	}
	
	public void run() throws Exception{
		EventLoopGroup group = new NioEventLoopGroup();
		try{
			Bootstrap b = new Bootstrap();
			b.group(group)
			 .channel(NioSocketChannel.class)
			 .handler(new ChannelInitializer<SocketChannel>(){

				@Override
				protected void initChannel(SocketChannel ch) throws Exception {
					ChannelPipeline pipe = ch.pipeline();
					pipe.addLast(new DelimiterBasedFrameDecoder(8192,  Delimiters.lineDelimiter()));
					pipe.addLast(new StringDecoder());
					pipe.addLast(new StringEncoder());
					pipe.addLast(new ChatClientHandler());
				}

			 });
			
			Channel channel = b.connect(host, port).sync().channel();
			BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
			while(true){
				channel.writeAndFlush(in.readLine()+"\r\n");
			}
			
		}finally{
			group.shutdownGracefully();
		}
	}
	
	public static void main(String args[]) throws Exception{
		new ChatClient("localhost", 8090).run();
	}
}

