"""
ctx_event_bus_advanced.py
-------------------------
Publishes and subscribes to events via a distributed message broker (e.g., Redis, RabbitMQ).
Distributed event bus for orchestrating events across multiple processes or nodes.

Usage:
    run(ctx, {"action": "publish", "event": "data_ready", "payload": {...}, "broker_url": "redis://localhost:6379/0"})
    run(ctx, {"action": "subscribe", "event": "data_ready", "broker_url": "redis://localhost:6379/0"})

Description:
    - Uses redis-py for Redis pub/sub.
"""
import redis
import json

def run(ctx, params):
    broker_url = params["broker_url"]
    r = redis.from_url(broker_url)
    if params["action"] == "publish":
        event = params["event"]
        payload = params.get("payload", {})
        r.publish(event, json.dumps(payload))
        return {"published": event}
    elif params["action"] == "subscribe":
        event = params["event"]
        pubsub = r.pubsub()
        pubsub.subscribe(event)
        # Blocking call: get one message
        for message in pubsub.listen():
            if message["type"] == "message":
                ctx[f"event_{event}"] = json.loads(message["data"])
                break
        return {"received": event}
    else:
        return {"status": "noop"}