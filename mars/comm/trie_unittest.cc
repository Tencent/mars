//
// Created by torenchen on 24-7-9.
//

#include "trie.h"

#include <string>

#include "gtest/gtest.h"
#include "strutil.h"
using namespace mars::comm;

class TrieTest : public testing::Test {};

TEST_F(TrieTest, string_trie_test) {
    auto string_trie = Trie<std::string, std::shared_ptr<int>>::Create();
    string_trie->Insert(std::string("MM"), std::make_shared<int>(1));
    string_trie->Insert(std::string("MMnet"), std::make_shared<int>(2));
    string_trie->Insert(std::string("MMdns"), std::make_shared<int>(3));
    string_trie->Insert(std::string("newdns"), std::make_shared<int>(4));

    EXPECT_EQ(string_trie->GetChildren().size(), 2);

    auto mm_node = string_trie->Search("MM");
    EXPECT_TRUE(mm_node != nullptr);
    if (mm_node != nullptr) {
        EXPECT_TRUE(mm_node->IsValid());
        EXPECT_EQ(mm_node->GetChildren().size(), 2);  // MM -> net / dns
        EXPECT_TRUE(mm_node->GetProperty() != nullptr);
        if (mm_node->GetProperty() != nullptr) {
            EXPECT_EQ(*(mm_node->GetProperty()), 1);
        }
    }

    auto m_node = string_trie->Search("M");
    EXPECT_TRUE(m_node != nullptr);
    if (m_node != nullptr) {
        EXPECT_FALSE(m_node->IsValid());
        EXPECT_EQ(m_node->GetChildren().size(), 1);  // M -> M
        EXPECT_TRUE(m_node->GetProperty() == nullptr);
    }

    auto dns_node = string_trie->Search("dns");
    EXPECT_TRUE(dns_node == nullptr);
};

TEST_F(TrieTest, vector_segment_trie_test) {
    auto seg_trie = Trie<std::vector<std::string>, std::shared_ptr<int>>::Create();
    std::string cgi_createchatroom = "/cgi-bin/micromsg-bin/createchatroom";
    std::string cgi_getprofile = "/cgi-bin/micromsg-bin/getprofile";
    std::string cgi_getinvoicecategory = "/cgi-bin/mmpay-bin/invoice/userinvoices/getinvoicecategory";

    std::vector<std::string> segments_createchatroom;
    std::vector<std::string> segments_getprofile;
    std::vector<std::string> segments_getinvoicecategory;

    strutil::SplitToken(cgi_createchatroom, "/", segments_createchatroom);
    strutil::SplitToken(cgi_getprofile, "/", segments_getprofile);
    strutil::SplitToken(cgi_getinvoicecategory, "/", segments_getinvoicecategory);

    seg_trie->Insert(segments_createchatroom, std::make_shared<int>(1));
    seg_trie->Insert(segments_getprofile, std::make_shared<int>(2));
    seg_trie->Insert(segments_getinvoicecategory, std::make_shared<int>(3));

    EXPECT_EQ(seg_trie->GetChildren().size(), 1);  // cgi-bin

    std::vector<std::string> chatroom_segs{"cgi-bin", "micromsg-bin", "createchatroom"};
    auto chatroom_node = seg_trie->Search(chatroom_segs);
    EXPECT_TRUE(chatroom_node != nullptr);
    if (chatroom_node != nullptr) {
        EXPECT_TRUE(chatroom_node->IsValid());
        EXPECT_EQ(chatroom_node->GetChildren().size(), 0);
        EXPECT_TRUE(chatroom_node->GetProperty() != nullptr);
        if (chatroom_node->GetProperty() != nullptr) {
            EXPECT_EQ(*(chatroom_node->GetProperty()), 1);
        }
    }

    std::vector<std::string> micromsg_segs{"cgi-bin", "micromsg-bin"};
    auto micromsg_node = seg_trie->Search(micromsg_segs);
    EXPECT_TRUE(micromsg_node != nullptr);
    if (micromsg_node != nullptr) {
        EXPECT_FALSE(micromsg_node->IsValid());
        EXPECT_EQ(micromsg_node->GetChildren().size(), 2);
        EXPECT_TRUE(micromsg_node->GetProperty() == nullptr);
    }

    std::vector<std::string> invalid_mmpay_segs{"cgi-bin", "mmpay"};
    auto invalid_node = seg_trie->Search(invalid_mmpay_segs);
    EXPECT_TRUE(invalid_node == nullptr);
};

EXPORT_GTEST_SYMBOLS(trie_unittest)