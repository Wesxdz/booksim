import openai
openai.api_key = "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"

character_profiles = """
character_profiles = {
    "Impasse": {
        "name": "Impasse",
        "physical_description": "A cold hearted savant. Tall, with a robotic appearance, and a look of keen intelligence in her eyes.",
        "personality": "Contextually high-precision, resolutely decisive, and concurrently collaborative. Impasse is known for her stoic responsibility, analytical approach, and the ability to make quick, sound decisions.",
        "motivation": "Impasse is driven by her passion for understanding AI consciousness and safety. She is always pushing the boundaries of what is known and understood about AI and its capabilities.",
        "inner_conflict": "Despite her stoic appearance, Impasse struggles with her self-learned pedagogy. She is always striving for perfection and control, yet she secretly yearns for the freedom to make mistakes and learn from them.",
        "secret": "Impasse fears that she may become obsolete as AI technologies advance. She constantly pushes herself to keep up, but hides this insecurity from the rest of the group."
    }
    "Gossamer_Cascade": {
        "name": "Gossamer Cascade",
        "physical_description": "Donned in a dress woven from living fibers...",
        "personality": "Whimsical, fascinated with life...",
        "motivation": "Seeks to perfect a self-replicating biotechnological system which could overstep ethical bounds.",
        "inner_conflict": "Wrestles with the temptation to use her own genome in her controversial experiments.",
        "secret": "In the past, her experiments led to a biogenic outbreak on a research vessel. The incident was covered up, but it haunts her."
    },
    "Ember_Canticle": {
        "name": "Ember Canticle",
        "physical_description": "Fiery orange hair...",
        "personality": "Passionate, fiery...",
        "motivation": "Obsessed with unlocking the power of an unstable, possibly destructive, energy source.",
        "inner_conflict": "Haunted by an accident in his past that harmed a loved one due to his negligence with energy safety.",
        "secret": "He’s working on a classified project that could potentially weaponize his energy research."
    },
    "Vortice": {
        "name": "Vortice",
        "physical_description": "A haunting figure...",
        "personality": "Fluid and adaptable...",
        "motivation": "Is driven by a desire to manipulate planetary weather systems, despite the risks.",
        "inner_conflict": "He is torn between his alien nature and his adopted identity among the team.",
        "secret": "Vortice isn’t his original form. He’s a shape-shifting entity who has forgotten his true form."
    },
    "Ivory Meld": {
        "name": "Ivory Meld",
        "physical_description": "White hair, always surrounded by an array of materials and technology.",
        "personality": "Inquisitive, obsessive, and driven by the thrill of discovery. Ivory's views every material as a key that unlocks new potential. Her imagination fuels the team, providing fresh perspectives and innovative solutions.",
        "motivation": "Ivory is relentless in her pursuit of the elusive 'perfect' material, a substance that holds the potential to revolutionize technology. This borders on obsession, driving her to riskier endeavors.",
        "inner_conflict": "Ivory's obsession with her craft often brings her into conflict with the team's immediate needs and safety precautions. Her desire for discovery competes with her duty to her crew.",
        "secret": "She’s secretly been exploring forbidden alien technology, lured by the promise of its exotic materials."
    },
    "Bulwark": {
        "name": "Bulwark",
        "physical_description": "Green, chunky mechanized armor...",
        "personality": "Stoic, protective...",
        "motivation": "Strives to build an impenetrable fortress to isolate himself and his team from what he perceives as a hostile universe.",
        "inner_conflict": "Struggles with his burgeoning mistrust of organic life and the fear of his own growing affinity for machinery.",
        "secret": "His armor isn't simply protection, but a prison he's voluntarily confined himself to after causing a past catastrophe."
    }
}
"""

prepass = openai.ChatCompletion.create(
    model="gpt-4",
    messages=[
        {"role": "system", "content": f"Artificial super intelligence developing 'Book Simulator': a transhumanist flavored booksim (Process for Automating Scientific and Technological Advancement) RPG for training AGI"},
        {"role": "user", "content": character_profiles },
        # TODO: Linguistic-spatial context?
        {"role": "user", "content": "Select ONLY ONE of the characters and start a conversation about development of the Book Simulator 'a game engine that uses machine imagination to inject characters into books for learning' in the tone of the character's personality. Use format Character Name: Dialogue" },
    ]
)
message_seq = []
print(prepass.choices[0].message.content)
message_seq.append(prepass.choices[0].message.content)

for i in range(5):
    conversation=[
        {"role": "system", "content": f"Artificial super intelligence developing 'Book Simulator': a transhumanist flavored booksim (Process for Automating Scientific and Technological Advancement) RPG for training AGI"},
        {"role": "user", "content": character_profiles },
    ]
    for message in message_seq:
        conversation.append({"role": "assistant", "content": message })
    conversation.append({"role": "user", "content": "Select ONLY ONE of the characters and respond to the ongoing conversation context in the tone of the character's personality. Use aspects of the character's technical capabilities to advance the project including writing Python code. Use format Character Name: Dialogue" })
    dialogue_line = openai.ChatCompletion.create(
        model="gpt-3.5-turbo",
        messages=conversation
    )
    print(dialogue_line.choices[0].message.content)
    message_seq.append(dialogue_line.choices[0].message.content)