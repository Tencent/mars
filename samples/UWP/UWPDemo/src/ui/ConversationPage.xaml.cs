using System;
using System.Collections.ObjectModel;
using UWPDemo.model;
using UWPDemo.scene;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace UWPDemo.ui
{

    public sealed partial class ConversationPage : Page, IMarsPushObserver
    {

        public ConversationPage()
        {
            this.InitializeComponent();
            NavigationCacheMode = NavigationCacheMode.Enabled;
            
            MarsPushMgr.addObserver((int)CgiCmdID.CgiCmdID_ConvList, this);
            this.DataContext = DataCore.getInstance();

            refreshList();
        }

        private async void readMeBtn_Click(object sender, RoutedEventArgs e)
        {
            readMeDialog.Title = "Mars Sample是一个快速聊天工具";
            readMeDialogText.Text = "请注意：\n" +
                            "   1. 消息在服务器和客户端均不做存储\n" +
                            "   2. 服务器仅将消息推送给与其保持长连接的客户端\n" +
                            "      * 当前长连接不可用时您将无法收到消息推送\n" +
                            "      * Mars Sample未运行时也将无法收到消息推送\n" +
                            "   3. 消息使用服务器处理时间排序，并非消息本身的时序";
            await readMeDialog.ShowAsync();
        }

        private void readMeConfirmBtn_Click(object sender, RoutedEventArgs e)
        {
            readMeDialog.Hide();
        }

        private void refreshListBtn_Click(object sender, RoutedEventArgs e)
        {
            refreshList();
        }

        private void refreshList()
        {
            showLoading();
            NSConvList scene = new NSConvList(0, "123456");
            scene.doScene();
        }

        private void showLoading()
        {
            progressRingCon.IsActive = true;
            progressRingCon.Visibility = Visibility.Visible;
            refreshListBtn.Visibility = Visibility.Collapsed;
            readMeBtn.Visibility = Visibility.Collapsed;
            conListView.Visibility = Visibility.Collapsed;
            errorTextBlock.Visibility = Visibility.Collapsed;
        }

        private void hideLoading(bool isShowErrorText)
        {
            progressRingCon.IsActive = false;
            progressRingCon.Visibility = Visibility.Collapsed;
            refreshListBtn.Visibility = Visibility.Visible;
            readMeBtn.Visibility = Visibility.Visible;
            if (isShowErrorText)
            {
                errorTextBlock.Visibility = Visibility.Visible;
                conListView.Visibility = Visibility.Collapsed;
            }
            else
            {
                errorTextBlock.Visibility = Visibility.Collapsed;
                conListView.Visibility = Visibility.Visible;
            }
        }

        private async void conListView_ItemClick(object sender, ItemClickEventArgs e)
        {
            if (String.IsNullOrWhiteSpace(DataCore.getInstance().UserName))
            {
                contentDialog.Title = "首次进入会话请输入昵称";
                await contentDialog.ShowAsync();
            }

            //pressed back button
            if (!String.IsNullOrWhiteSpace(DataCore.getInstance().UserName))
            {
                LocalConversation con = e.ClickedItem as LocalConversation;
                Frame root = Window.Current.Content as Frame;
                root.Navigate(typeof(ChatPage), con);
            }
        }

        private void confirmBtn_Click(object sender, RoutedEventArgs e)
        {
            trySaveNameAndHideDialog();
        }

        private void nameTextBox_KeyDown(object sender, Windows.UI.Xaml.Input.KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                trySaveNameAndHideDialog();
            }
        }

        private void trySaveNameAndHideDialog()
        {
            if (!String.IsNullOrWhiteSpace(nameTextBox.Text))
            {
                DataCore.getInstance().UserName = nameTextBox.Text;
                contentDialog.Hide();
            }
        }

        public void onPush(int cmdID, MarsEventArgs args)
        {
            if (cmdID == (int)CgiCmdID.CgiCmdID_ConvList && args != null && args.Code == EventConst.SUCCESS)
            {
                //成功
                hideLoading(false);
                ObservableCollection<LocalConversation> resultList = (ObservableCollection<LocalConversation>)args.Data;
                DataCore.getInstance().setConversationList(resultList);
            }
            else
            {
                //失败
                hideLoading(true);
                DataCore.getInstance().clearConversationList();
            }
        }
        
    }
}
