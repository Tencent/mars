using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UWPDemo.model;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace UWPDemo.ui
{
    public class MessageDataTemplateSelector : DataTemplateSelector
    {
        public DataTemplate messageComeDataTemplate { get; set; }

        public DataTemplate messageSelfDataTemplate { get; set; }

        protected override DataTemplate SelectTemplateCore(object item, DependencyObject container)
        {
            if (item is ChatMsg)
            {
                if ((item as ChatMsg).IsComMeg)
                {
                    return messageComeDataTemplate;
                }
                else
                {
                    return messageSelfDataTemplate;
                }
            }

            return base.SelectTemplateCore(item);
        }
    }
}
