#!/usr/bin/env python
# coding:utf-8
# author: dj


import google.protobuf.text_format

class ProtocolBufferMixin(object):
    """Protocol Buffer support for RequestHandler objects."""

    MIMETYPE = 'application/octet-stream'
    # 'application/x-protobuf'

    def read_protobuf(self, message_type, data):
        """Attempts to parse a protocol buffer message from the given data."""
        # Create the message object and attempt to parse the data into it.
        try:
            message = message_type()
            print self.request.headers
            if self.request.headers.get('Content-Type', None) == self.MIMETYPE:
                message.ParseFromString(data)
            else:
                google.protobuf.text_format.Merge(data, message)
        except Exception, e:
            logging.error("Failed to parse %s (%d bytes)" %
                    (message_type.__name__, len(data)), exc_info=True)
            self.set_status(400)
            return None

        # Also make sure the message is fully initialized (i.e. all required
        # fields have been filled out).
        if not message.IsInitialized():
            logging.error("Incomplete data for %s (%d bytes)" %
                    (message_type.__name__, len(data)))
            self.set_status(400)
            return None

        # At this point, we have a fully-initialized message object.
        return message

    def write_protobuf(self, message, format='text/plain'):
        """Writes a protocol buffer message to the output buffer.

        If the client's Accept: header advertises support for the protocol
        buffer MIME type, we write the message in binary form.  Otherwise, we
        use the protocol buffer text format.
        """
        if self.MIMETYPE == format: #self.MIMETYPE in self.request.headers:
            self.set_header('Content-Type', self.MIMETYPE)
            self.write(message.SerializeToString())
        else:
            self.set_header('Content-Type', 'text/plain')
            text = google.protobuf.text_format.MessageToString(message)
            self.write(text)