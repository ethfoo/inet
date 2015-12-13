import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Date;
import java.util.Iterator;
import java.util.Set;

public class TimeServer implements Runnable{
	
	private Selector selector;
	private ServerSocketChannel servChannel;
	
	
	public TimeServer(int port){
		try {
			selector = Selector.open();
			servChannel = ServerSocketChannel.open();
			servChannel.configureBlocking(false);
			servChannel.socket().bind(new InetSocketAddress(port), 1024);
			servChannel.register(selector, SelectionKey.OP_ACCEPT);
			
			
		} catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		}
	}
	
	public void run() {
		while(true){
			
			try {
				selector.select(1000);
				Set<SelectionKey> selectedKeys = selector.selectedKeys();
				Iterator<SelectionKey> it = selectedKeys.iterator();
				
				SelectionKey key = null;
				while(it.hasNext()){
					key = it.next();
					it.remove();
				
					try{
						handleEvents(key);
					}catch(Exception e){
						if(key != null){
							key.cancel();
							if(key.channel() != null){
								key.channel().close();
							}
						}
					}
				}
				
				
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		
	}
	
	private void handleEvents(SelectionKey key) throws IOException{
		if( key.isValid() ){
			if( key.isAcceptable() ){
				ServerSocketChannel ssc = (ServerSocketChannel) key.channel();
				SocketChannel sc = ssc.accept();
				sc.configureBlocking(false);
				sc.register(selector, SelectionKey.OP_READ);
			}
			if( key.isReadable() ){
				SocketChannel sc = (SocketChannel) key.channel();
				ByteBuffer readBuffer = ByteBuffer.allocate(1024);
				int readBytes = sc.read(readBuffer);
				if( readBytes >0 ){
					readBuffer.flip();
					byte[] bytes = new byte[readBuffer.remaining()];
					String body = new String(bytes, "UTF-8");
					String currentTime = "QUERY TIME".equalsIgnoreCase(body)?
							new Date(System.currentTimeMillis()).toString():"BAD REQUEST";
					doWrite(sc, currentTime);		
				}else if( readBytes <0 ){
					key.cancel();
					key.channel().close();
				}
				
			}
		}
	}

	private void doWrite(SocketChannel channel, String response) throws IOException{
		if(response != null && response.trim().length()>0){
			byte[] bytes = response.getBytes();
			ByteBuffer writeBuffer = ByteBuffer.allocate(bytes.length);
			writeBuffer.put(bytes);
			writeBuffer.flip();
			channel.write(writeBuffer);
			
		}
		
	}
	
	
	public static void main(String[] args) {

		int port = 1024;
		
		TimeServer timeServer = new TimeServer(port);
		new Thread(timeServer).start();
	}
	
}

