Module Module1

    Sub Main()
        Dim con As safmq.net.MQConnection
        Dim err As safmq.net.ErrorCode
        Dim q As safmq.net.QueueHandle
        Dim msg As safmq.net.QueueMessage
        Dim w As System.IO.StreamWriter


        con = safmq.net.MQBuilder.BuildConnection(New Uri("safmq://localhost"), "admin", "")

        q = New safmq.net.QueueHandle
        err = con.OpenQueue("testQ", q)
        Console.WriteLine("OpenQueue: " & err)

        msg = New safmq.net.QueueMessage
        msg.MessagePriority = safmq.net.MessagePriorityEnum.MP_HIGH
        msg.Label = "Test Hello!"
        w = New System.IO.StreamWriter(msg.Stream)
        w.WriteLine("Hello World!")
        w.Flush()

        err = con.Enqueue(q, msg)
        Console.WriteLine("Enqueue: " & err)
        err = con.Close
        Console.WriteLine("Close: " & err)

    End Sub

End Module
