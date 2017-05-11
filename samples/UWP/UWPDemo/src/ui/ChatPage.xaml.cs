using System;
using UWPDemo.model;
using UWPDemo.scene;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace UWPDemo.ui
{
    public sealed partial class ChatPage : Page
    {
        private LocalConversation mConversation;
        private DispatcherTimer mTimer;
        private static object sTimerLocker = new object();

        public ChatPage()
        {
            this.InitializeComponent();
            this.DataContext = DataCore.getInstance();
            InputPane.GetForCurrentView().Showing += inputShowing;
            InputPane.GetForCurrentView().Hiding += inputHiding;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            if (e != null && e.Parameter != null)
            {
                mConversation = (LocalConversation)e.Parameter;
                DataCore.getInstance().loadMsgs(mConversation.ConId);
                titleText.Text = mConversation.ConNotice;
            }
        }

        private void inputShowing(InputPane sender, InputPaneVisibilityEventArgs args)
        {
            double newHeight = this.rowChat.ActualHeight - args.OccludedRect.Height;
            if (newHeight > 0)
            {
                this.rowChat.Height = new GridLength(newHeight);
                tryScrolltoLast(50);
            }
        }

        private void inputHiding(InputPane sender, InputPaneVisibilityEventArgs args)
        {
            this.rowChat.Height = new GridLength(1, GridUnitType.Star);
        }
    
        private void inputText_GotFocus(object sender, RoutedEventArgs e)
        {
            bottom.Background = new SolidColorBrush { Color = Windows.UI.Color.FromArgb(255, 255, 255, 255) };
        }

        private void inputText_LostFocus(object sender, RoutedEventArgs e)
        {
            bottom.Background = new SolidColorBrush { Color = Windows.UI.Color.FromArgb(255, 247, 247, 247) };
        }

        private void inputText_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                sendMsg();
            }
        }

        private void sendBtn_Click(object sender, RoutedEventArgs e)
        {
            sendMsg();
        }

        private void sendMsg()
        {
            if (!String.IsNullOrWhiteSpace(DataCore.getInstance().UserName) && mConversation != null && !String.IsNullOrWhiteSpace(mConversation.ConId) && !String.IsNullOrWhiteSpace(inputText.Text))
            {
                ChatMsg msg = new ChatMsg { From = DataCore.getInstance().UserName, ConversationId = mConversation.ConId, Message = inputText.Text, Date = DateTime.Now.ToString(), IsComMeg = false };
                DataCore.getInstance().addMsg(msg);
                inputText.Text = "";
                tryScrolltoLast(50);

                NSSendmsg scene = new NSSendmsg("123456", DataCore.getInstance().UserName, mConversation.ConId, msg.Message);
                scene.doScene();
            }
        }

        private void chatListView_ContainerContentChanging(ListViewBase sender, ContainerContentChangingEventArgs args)
        {
            tryScrolltoLast(50);
        }

        private void tryScrolltoLast(int delay)
        {
            lock (sTimerLocker)
            {
                if (mTimer == null)
                {
                    mTimer = new DispatcherTimer();
                    mTimer.Tick += mTimer_Tick;
                }
                else
                {
                    mTimer.Stop();
                }
                mTimer.Interval = TimeSpan.FromMilliseconds(delay);
                mTimer.Start();
            }
        }

        private void mTimer_Tick(object sender, object e)
        {
            lock (sTimerLocker)
            {
                if (mTimer != null)
                {
                    mTimer.Stop();
                }
            }

            Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                chatScrollViewer.ChangeView(null, chatScrollViewer.ExtentHeight, null, false);
            });
        }
    }
}
