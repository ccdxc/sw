import pdb
from agent_config_mgr import cfg_meta_mapper, MessageModifer

#Whether to completely pass through
FULL_PASSTHROUGH = False

#Has Agent configured HAL, for testing only.
AGENT_CONFIGURED = False

def split_repeated_messages(message):
    requests = getattr(message, 'request')
    messages = []
    for request in requests:
        sub_message = type(message)()
        sub_message.request.extend([request])
        messages.append(sub_message)
    return messages

def combine_repeated_messages(curr_message, resp_message, msg_type):
    response = getattr(curr_message, msg_type)
    getattr(resp_message, msg_type).extend(response)
    return resp_message

#Main function handling all the GRPC messages from DOL side.
def CreateConfigFromDol(incoming_message, request_method_name):
	if FULL_PASSTHROUGH:
		return None, True
	
	object_helper = None
	try:
		object_helper = cfg_meta_mapper.dol_message_map[type(incoming_message).__name__]
	except KeyError:
		pass

	messages = split_repeated_messages(incoming_message)
	resp_message = None
	err = False
	msg_type = None
	for message in messages:
			if object_helper:
				#If we have a helper,
				#It means, Agent has already configured this.
				#Add config to dictionary
				object_helper.AddDolConfigMessage(message)
				#Create a response to Dol as well.
				curr_message = object_helper.GetResponseMessage(message)
				msg_type = 'response'
			else:
				#Modify message keys.
				curr_message = MessageModifer(message)
				msg_type = 'request'
				
			if not resp_message:
				resp_message = curr_message
			else:
				# Combine all the repeated messages, as DOL expects this.
				resp_message = combine_repeated_messages(curr_message, resp_message,
														msg_type)
	if not AGENT_CONFIGURED:
		return None, True
	return resp_message, err	